#pragma once

#include <MyTonic.h>
#include <core/utils/visitor.h>
#include <core/audio/registers.h>
#include <cstdint>
#include <string>

namespace GBEmulator
{
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

    private:
        void SetFrequency();
        void SetVolume();
        void Restart();

        MyWave m_wave;
        WaveVolumeRegister m_volumeReg;
        FrequencyHighRegister m_freqMsbReg;
        uint8_t m_soundLength = 0x00;
        uint16_t m_freq = 0x0000;

        bool m_enabled = false;

        size_t m_nbUpdateCalls = 0;
        uint8_t m_lengthCounter = 0x00;
    };
}