#include <exe/audio/audioSystem.h>

using GBEmulatorExe::AudioSystem;

int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    AudioSystem* audioSystem = reinterpret_cast<AudioSystem*>(userData);
    return audioSystem->RenderCallback(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, userData);   
}

AudioSystem::AudioSystem(unsigned nbChannels, unsigned sampleRate, unsigned bufferFrames)
    : m_nbChannels(nbChannels)
    , m_sampleRate(sampleRate)
    , m_bufferFrames(bufferFrames)
    // , m_messageService(*this)
{
    // DispatchMessageServiceSingleton::GetInstance().Connect(&m_messageService);
}

AudioSystem::~AudioSystem()
{
    // DispatchMessageServiceSingleton::GetInstance().Disconnect(&m_messageService);
    Shutdown();
}

bool AudioSystem::Initialize()
{
    if (m_initialized)
        return true;

    m_dac = new RtAudio();
    RtAudio::StreamParameters rtParams;
    rtParams.deviceId = m_dac->getDefaultOutputDevice();
    rtParams.nChannels = m_nbChannels;

    RtAudio::StreamOptions rtOptions;
    rtOptions.numberOfBuffers = 4;

    auto res = m_dac->openStream(&rtParams, NULL, RTAUDIO_FLOAT32, m_sampleRate, &m_bufferFrames, &audioCallback, this, &rtOptions);

    if (res != RtAudioErrorType::RTAUDIO_NO_ERROR)
    {
        std::cerr << "Error while opening audio stream. Error n" << int(res) << std::endl;
        Shutdown();
        return false;
    }

    if (m_enabled)
    {
        res = m_dac->startStream();

        if (res != RtAudioErrorType::RTAUDIO_NO_ERROR)
        {
            std::cerr << "Error while starting audio stream. Error n" << int(res) << std::endl;
            Shutdown();
            return false;
        }
    }

    return true;
}

bool AudioSystem::Shutdown()
{
    if (m_dac != nullptr && m_enabled)
    {
        m_dac->closeStream();
    }

    delete m_dac;
    m_dac = nullptr;

    return true;
}

void AudioSystem::Enable(bool value)
{
    if (value != m_enabled)
    {
        m_enabled = value;
        if (m_dac == nullptr)
            return;
        
        if (m_enabled)
            m_dac->startStream();
        else
            m_dac->stopStream();
    }
}