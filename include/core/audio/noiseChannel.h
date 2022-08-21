#pragma once

#include <MyTonic.h>
#include <gbNoise.h>
#include <core/utils/visitor.h>
#include <core/audio/registers.h>
#include <cstdint>
#include <string>

namespace GBEmulator
{
    class NoiseOscillator
    {
    public:
        NoiseOscillator();
        void Reset();
        void SetFrequency(double freq);

        double GetSample();

        double m_elaspedTime = 0.0;
        double m_volume = 0.0;
        uint16_t m_shiftRegister = 1;
        uint8_t m_shiftRegLength = 15;
        double m_realSampleDuration = 0.0;
        double m_sampleDuration = 0.0;
    };


    class NoiseChannel
    {
    public:
        NoiseChannel(Tonic::Synth& synth);
        ~NoiseChannel() = default;

        Tonic::Generator& GetWave() { return m_noise; }

        void Update(Tonic::Synth& synth);
        void Reset();

        bool IsEnabled() const { return m_enabled; }
        void SetEnable(bool enabled) { m_enabled = enabled; }

        void WriteByte(uint16_t addr, uint8_t data);
        uint8_t ReadByte(uint16_t addr) const;

        void SerializeTo(Utils::IWriteVisitor& visitor) const;
        void DeserializeFrom(Utils::IReadVisitor& visitor);

        double GetSample() { return m_oscillator.GetSample(); }

    private:
        void SetFrequency();
        void SetVolume();

        MyNoise m_noise;
        WavePatternRegister m_lengthReg;
        VolumeEnveloppeRegister m_volumeReg;
        FrequencyHighRegister m_initialReg;
        PolynomialCounterRegister m_polyReg;

        bool m_enabled = false;

        NoiseOscillator m_oscillator;

        size_t m_nbUpdateCalls = 0;
        uint8_t m_lengthCounter = 0x00;
        uint8_t m_volumeCounter = 0x00;
    };
}