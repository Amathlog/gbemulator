#pragma once

#include <Tonic/TonicCore.h>
#include <Tonic/Generator.h>


class MyWave_ : public Tonic::Tonic_::Generator_
{
    protected:
        void computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context );
        unsigned nbSamplesPerRandomValue = 1;
        float volume = 0.0f;
        uint16_t m_shiftRegister = 1;
        uint8_t m_shiftRegLength = 15;

    public:
        void reset() { m_shiftRegister = 1; volume = 0.0f; nbSamplesPerRandomValue = 1;}

        void setFreq(float value)
        {
            nbSamplesPerRandomValue = 0;
            if (value != 0.0f)
            {
                double sampleDuration = 1.0 / Tonic::sampleRate();
                double signalPeriod = 1.0 / value;
                nbSamplesPerRandomValue = (unsigned)(floor(signalPeriod / sampleDuration));
            }

            if (nbSamplesPerRandomValue == 0)
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

inline void MyWave_::computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context )
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
            value = m_shiftRegister & 0x0001 ? volume : 0.0f;
        }
        *fdata++ = value;
    }
}

class MyWave : public Tonic::TemplatedGenerator<MyWave_>
{
public:
    void setFreq(float value) { gen()->setFreq(value); }
    void setVolume(float value) { gen()->setVolume(value); }
    void setLengthCounter(bool isLong) { gen()->setLengthCounter(isLong); }
    void reset() { gen()->reset(); }
};