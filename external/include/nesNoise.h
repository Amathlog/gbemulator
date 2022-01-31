#include <Tonic/TonicCore.h>
#include <Tonic/Generator.h>


class MyNoise_ : public Tonic::Tonic_::Generator_
{
    protected:
        void computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context );
        unsigned nbSamplesPerRandomValue = 1;
        float volume = 0.0f;
        uint16_t m_shiftRegister = 1;

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
            m_shiftRegister = (feedback << 14) | (m_shiftRegister >> 1);
            value = m_shiftRegister & 0x0001 ? volume : 0.0f;
        }
        *fdata++ = value;
    }
}

class MyNoise : public Tonic::TemplatedGenerator<MyNoise_>
{
public:
    void setFreq(float value) { gen()->setFreq(value); }
    void setVolume(float value) { gen()->setVolume(value); }
    void reset() { gen()->reset(); }
};