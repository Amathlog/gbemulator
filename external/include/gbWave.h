#pragma once

#include <Tonic/TonicCore.h>
#include <Tonic/Generator.h>
#include <array>
#include <cstring>


class MyWave_ : public Tonic::Tonic_::Generator_
{
    protected:
        void computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context );
        std::atomic<unsigned> nbSamplesPerValue = 1;
        std::atomic<float> volume = 0.0f;
        std::atomic<uint8_t> ptr = 0x00;
        std::array<uint8_t, 32> data;
        std::array<uint8_t, 32> newData;
        std::atomic<bool> hasNewData = false;
        double oneSampleDuration;
        std::atomic<double> newSampleDuration;
        std::atomic<bool> hasNewSampleDuration = false;
        double elapsedTime = 0.0;

    public:
        void reset() { data.fill(0); volume = 0.0f; nbSamplesPerValue = 1; resetPosition(); elapsedTime = 0.0; ptr = 0; }

        void setFreq(float value)
        {
            if (value != 0.0f)
            {
                double signalPeriod = 1.0 / value;

                newSampleDuration = (signalPeriod / 32.0);
                hasNewSampleDuration = true;
            }
            else
            {
                oneSampleDuration = 0.0;
                volume = 0.0f;
            }
        }

        void setVolume(float value)
        {
            volume = value;
        }

        void setSample(uint8_t sample, uint8_t position)
        {
            position = std::min<uint8_t>(position, 31);
            newData[position] = sample;
            hasNewData = true;
        }

        uint8_t getSample(uint8_t position) const
        {
            position = std::min<uint8_t>(position, 31);
            return data[position];
        }

        void resetPosition()
        {
            ptr = 0;
        }
};

inline void MyWave_::computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context )
{
    TonicFloat* fdata = &outputFrames_[0];
    float value = 0.0f;
    double realSampleDuration = 1.0 / Tonic::sampleRate();
    for (unsigned int i=0; i<outputFrames_.size(); i++)
    {
        value = volume * (float)(data[ptr]) / 0x0F;

        if (volume > 0.0f)
        {
            elapsedTime += realSampleDuration;
            if (elapsedTime >= oneSampleDuration)
            {
                ptr = (ptr + 1) % (uint8_t)(data.size());
                elapsedTime -= oneSampleDuration;

                if (ptr == 0)
                {
                    if (hasNewData)
                    {
                        data = newData;
                        hasNewData = false;
                    }

                    if (hasNewSampleDuration)
                    {
                        oneSampleDuration = newSampleDuration;
                        hasNewSampleDuration = false;
                    }
                }
            }
        }
        *fdata++ = value;
    }
}

class MyWave : public Tonic::TemplatedGenerator<MyWave_>
{
public:
    void setFreq(float value) { gen()->setFreq(value); }
    void setVolume(float value) { gen()->setVolume(value); }
    void setSample(uint8_t sample, uint8_t position) { gen()->setSample(sample, position); }
    void reset() { gen()->reset(); }
    void resetPosition() { gen()->resetPosition(); }
    uint8_t getSample(uint8_t position) const { return const_cast<MyWave*>(this)->gen()->getSample(position); }
};