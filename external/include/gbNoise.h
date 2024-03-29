#pragma once

#include <Tonic/TonicCore.h>
#include <Tonic/Generator.h>
#include <atomic>


class MyNoise_ : public Tonic::Tonic_::Generator_
{
    protected:
        void computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context );
        std::atomic<unsigned> nbSamplesPerRandomValue = 1;
        std::atomic<float> volume = 0.0f;
        std::atomic<uint16_t> m_shiftRegister = 1;
        std::atomic<uint8_t> m_shiftRegLength = 15;

    public:
        void reset() { m_shiftRegister = 1; volume = 0.0f; nbSamplesPerRandomValue = 1;}

        void setFreq(float value)
        {
            if (value != 0.0f)
            {
                double sampleDuration = 1.0 / Tonic::sampleRate();
                double signalPeriod = 1.0 / value;
                unsigned newValue = (unsigned)(floor(signalPeriod / sampleDuration));
                if (newValue != 0)
                {
                    nbSamplesPerRandomValue = newValue;
                }
                else
                {
                    nbSamplesPerRandomValue = 1;
                    volume = 0.0f;
                }
            }
            else
            {
                nbSamplesPerRandomValue = 1;
                volume = 0.0f;
            }
        }

        void setVolume(float value)
        {
            volume = value;
        }

        void setLengthCounter(bool isLong)
        {
            m_shiftRegLength = isLong ? 15 : 7;
        }
};

inline void MyNoise_::computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context )
{
    TonicFloat* fdata = &outputFrames_[0];
    float value = 0.0f;
    for (unsigned int i=0; i<outputFrames_.size(); i++)
    {
        if (i % nbSamplesPerRandomValue == 0)
        {
            uint16_t otherFeedback = (m_shiftRegister >> 1) & 0x0001;
            uint16_t feedback = (m_shiftRegister ^ otherFeedback) & 0x0001;
            m_shiftRegister = (feedback << (m_shiftRegLength - 1)) | (m_shiftRegister >> 1);
            value = m_shiftRegister & 0x0001 ? (float)volume : (float)(-volume);
        }
        *fdata++ = value;
    }
}

class MyNoise : public Tonic::TemplatedGenerator<MyNoise_>
{
public:
    void setFreq(float value) { gen()->setFreq(value); }
    void setVolume(float value) { gen()->setVolume(value); }
    void setLengthCounter(bool isLong) { gen()->setLengthCounter(isLong); }
    void reset() { gen()->reset(); }
};