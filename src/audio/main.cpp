#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstring>
#include <thread>
#include <chrono>
#include <cmath>

class RtAudioError : public std::exception{  
    public:  
        const char * what() const throw()  
        {  
            return "Audio error\n";  
        }  

        void printMessage()
        {
            std::cout << what();
        }
}; 

constexpr double pi = 3.141592653589793;
constexpr double two_pi = 2.0 * pi;
constexpr unsigned nbChannels = 2;
constexpr unsigned sampleRate = 44100;

static bool stop = false;
static bool headStart = false;

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer(size_t maxSize) 
        : m_maxSize(maxSize)
    {
        m_buffer.resize(maxSize);
    }

    void ReadData(void* outData, size_t bytesSize)
    {
        size_t nbElements = bytesSize / sizeof(T);
        bool said = false;

        while (!m_stop && nbElements > m_nbReadableSamples)
        {
            if (!said)
            {
                std::cout << "Reader too fast" << std::endl;
                said = false;
            }
            // Waiting
        }

        if (m_stop)
        {
            return;
        }

        //std::scoped_lock lock(m_lock);
        size_t targetPtr = m_readerPtr + nbElements;

        if (targetPtr >= m_maxSize)
        {
            size_t tempNbElements = (m_maxSize - m_readerPtr);
            size_t tempSize = tempNbElements * sizeof(T);
            std::memcpy(outData, m_buffer.data() + m_readerPtr, tempSize);
            targetPtr -= m_maxSize;
            outData = ((char*)outData) + tempSize;
            bytesSize -= tempSize;
            m_readerPtr = 0;
            m_nbReadableSamples -= tempNbElements;
            nbElements -= tempNbElements;
        }

        std::memcpy(outData, m_buffer.data() + m_readerPtr, bytesSize);
        m_nbReadableSamples -= nbElements;
        m_readerPtr += nbElements;
    }

    void WriteData(const void* inData, size_t bytesSize)
    {
        size_t nbElements = bytesSize / sizeof(T);

        while (!m_stop && nbElements + m_nbReadableSamples > m_maxSize)
        {
            // Waiting
        }

        if (m_stop)
        {
            return;
        }

        //std::scoped_lock lock(m_lock);
        size_t targetPtr = m_writerPtr + nbElements;

        if (targetPtr >= m_maxSize)
        {
            size_t tempNbElements = (m_maxSize - m_writerPtr);
            size_t tempSize = tempNbElements * sizeof(T);
            std::memcpy(m_buffer.data() + m_writerPtr, inData, tempSize);
            targetPtr -= m_maxSize;
            inData = ((const char*)inData) + tempSize;
            bytesSize -= tempSize;
            m_writerPtr = 0;
            m_nbReadableSamples += tempNbElements;
            nbElements -= tempNbElements;
        }

        std::memcpy(m_buffer.data() + m_writerPtr, inData, bytesSize);
        m_writerPtr += nbElements;
        m_nbReadableSamples += nbElements;

        if (m_nbReadableSamples > m_maxSize)
        {
            std::cout << "Writer is writing faster than the reader reads" << std::endl;
        }
    }

    void Stop() { m_stop = true; }

private:
    mutable std::mutex m_lock;
    size_t m_maxSize;
    std::vector<T> m_buffer;

    std::atomic<bool> m_stop = false;
    std::atomic<size_t> m_nbReadableSamples = 0;

    size_t m_writerPtr = 0;
    size_t m_readerPtr = 0;
};

struct UserData
{
    double frequency = 440.0;
    CircularBuffer<double>* buffer = nullptr;
};

using CallbackRtAudio = int(*)(void*, void*, unsigned int, double, RtAudioStreamStatus, void*);

// Two-channel sawtooth wave generator.
int saw(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
        double streamTime, RtAudioStreamStatus status, void *userData)
{
    unsigned int i, j;
    double *buffer = (double *)outputBuffer;
    double *lastValues = (double *)userData;
    if (status)
        std::cout << "Stream underflow detected!" << std::endl;
    // Write interleaved audio data.
    for (i = 0; i < nBufferFrames; i++)
    {
        for (j = 0; j < 2; j++)
        {
            *buffer++ = lastValues[j];
            lastValues[j] += 0.005 * (j + 1 + (j * 0.1));
            if (lastValues[j] >= 1.0)
                lastValues[j] -= 2.0;
        }
    }
    return 0;
}

int MySin(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    static double phase = 0.0;

    double frequency = *(double*)userData;

    for (unsigned i = 0; i < nBufferFrames; ++i)
    {
        phase += two_pi * frequency / sampleRate;
        if (phase >= two_pi)
            phase -= two_pi;

        double value = std::sin(phase);

        for (int j = 0; j < nbChannels; ++j)
        {
            ((double*)outputBuffer)[nbChannels * i + j] = value;
        }
    }

    return 0;
}

int MySquare(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    static double phase = 0.0;

    double frequency = *(double*)userData;

    for (unsigned i = 0; i < nBufferFrames; ++i)
    {
        phase += frequency / sampleRate;

        while (phase > 1.0)
            phase -= 1.0f;

        while (phase < 0.0)
            phase += 1.0;

        double value = 0.0;
        if (phase <= 0.5)
            value = -1.0;
        else
            value =  1.0;

        for (int j = 0; j < nbChannels; ++j)
        {
            ((double*)outputBuffer)[nbChannels * i + j] = value;
        }
    }

    return 0;
}

int ReadBuffer(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
    double streamTime, RtAudioStreamStatus status, void* userData)
{
    CircularBuffer<double>& buffer = *((UserData*)userData)->buffer;

    buffer.ReadData((double*)outputBuffer, nBufferFrames * nbChannels * sizeof(double));

    return 0;
}

void WriteBuffer(CallbackRtAudio callback, UserData* UserData)
{
    bool firstTime = true;
    while (!stop)
    {
        // Producing 0.1s of sound then wait 0.1 seconds
        auto startTime = std::chrono::high_resolution_clock::now();
        constexpr unsigned nbSamplesToProduce = sampleRate / 10;
        double* tempBuffer = new double[nbSamplesToProduce * nbChannels];

        double frequency = UserData->frequency;

        callback(tempBuffer, nullptr, nbSamplesToProduce, 0.0, 0, &frequency);

        UserData->buffer->WriteData(tempBuffer, nbSamplesToProduce * nbChannels * sizeof(double));

        if (!firstTime)
            std::this_thread::sleep_for(std::chrono::milliseconds(90));

        firstTime = false;
    }
}

int main()
{
    RtAudio dac;
    if (dac.getDeviceCount() < 1)
    {
        std::cout << "\nNo audio devices found!\n";
        exit(0);
    }
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = nbChannels;
    parameters.firstChannel = 0;
    unsigned int bufferFrames = 256; // 256 sample frames

    CircularBuffer<double> buffer(1000000);

    UserData userData{ 440.0, &buffer };

    try
    {
        dac.openStream(&parameters, NULL, RTAUDIO_FLOAT64,
                       sampleRate, &bufferFrames, &ReadBuffer, (void *)&userData);
        dac.startStream();
    }
    catch (RtAudioError &e)
    {
        e.printMessage();
        exit(0);
    }

    std::thread writeThread(WriteBuffer, MySin, &userData);

    bool discardStop = false;
    while (!stop)
    {
        char input;
        std::cout << "\nPlaying ... press <enter> to quit.\n";
        std::cin.get(input);

        if (input == 'a')
        {
            // A4 or A5
            userData.frequency = userData.frequency == 440.0 ? 880.0 : 440.0;
            discardStop = true;
        }
        else
        {
            stop = !discardStop;
            discardStop = false;
        }
    }

    buffer.Stop();

    try
    {
        // Stop the stream
        dac.stopStream();
    }
    catch (RtAudioError &e)
    {
        e.printMessage();
    }
    if (dac.isStreamOpen())
        dac.closeStream();

    writeThread.join();

    return 0;
}