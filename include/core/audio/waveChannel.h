#pragma once

#include <core/audio/registers.h>
#include <core/utils/visitor.h>

#include <cstdint>

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
    double m_sampleDuration = 0.0;
    double m_elaspedTime = 0.0;
    double m_volume = 0.0;
    double m_realSampleDuration = 0.0;
    uint8_t m_ptr = 0;
};

class WaveChannel
{
public:
    WaveChannel() = default;
    ~WaveChannel() = default;

    void Update();
    void Reset();

    bool IsEnabled() const { return m_enabled; }
    void SetEnable(bool enabled) { m_enabled = enabled; }

    void WriteByte(uint16_t addr, uint8_t data);
    uint8_t ReadByte(uint16_t addr) const;

    void SerializeTo(Utils::IWriteVisitor& visitor) const;
    void DeserializeFrom(Utils::IReadVisitor& visitor);

    double GetSample();

private:
    void SetFrequency();
    void SetVolume();
    void Restart();

    WaveVolumeRegister m_volumeReg;
    FrequencyHighRegister m_freqMsbReg;
    uint16_t m_soundLength = 0x0000;
    uint16_t m_freq = 0x0000;

    WaveOscillator m_oscillator;

    bool m_enabled = false;
    bool m_DAC = false;

    size_t m_nbUpdateCalls = 0;
    uint16_t m_lengthCounter = 0x0000;
};
} // namespace GBEmulator