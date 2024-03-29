#pragma once

#include <MyTonic.h>
#include <core/utils/visitor.h>
#include <core/audio/registers.h>
#include <cstdint>
#include <string>

namespace GBEmulator
{
    class WaveOscillator
    {
    public:
        WaveOscillator();

        void Reset();
        double GetSample();

        void SetFrequency(double freq);

        std::array<uint8_t, 32> m_data;
        uint8_t m_ptr = 0;
        double m_sampleDuration = 0.0;
        double m_elaspedTime = 0.0;
        double m_volume = 0.0;
        double m_realSampleDuration = 0.0;
    };

    class WaveChannel
    {
    public:
        WaveChannel(Tonic::Synth& synth);
        ~WaveChannel() = default;

        Tonic::Generator& GetWave() { return m_wave; }

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
        void Restart();

        MyWave m_wave;
        WaveVolumeRegister m_volumeReg;
        FrequencyHighRegister m_freqMsbReg;
        uint16_t m_soundLength = 0x0000;
        uint16_t m_freq = 0x0000;

        WaveOscillator m_oscillator;

        bool m_enabled = false;

        size_t m_nbUpdateCalls = 0;
        uint16_t m_lengthCounter = 0x0000;
    };
}