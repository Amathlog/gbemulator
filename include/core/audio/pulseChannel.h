#pragma once

#include <MyTonic.h>
#include <core/utils/visitor.h>
#include <core/audio/registers.h>
#include <cstdint>
#include <string>

namespace GBEmulator 
{
    class PulseChannel
    {
    public:
        PulseChannel(Tonic::Synth& synth, int number);
        ~PulseChannel() = default;

        Tonic::Generator& GetWave() { return m_wave; }

        void Update(Tonic::Synth& synth);
        void Reset();

        bool IsEnabled() const { return m_enabled; }
        void SetEnable(bool enabled) { m_enabledChanged = enabled != m_enabled; m_enabled = enabled; }

        void WriteByte(uint16_t addr, uint8_t data);
        uint8_t ReadByte(uint16_t addr) const;

        void SerializeTo(Utils::IWriteVisitor& visitor) const;
        void DeserializeFrom(Utils::IReadVisitor& visitor);

    private:
        void UpdateFreq();
        void CheckOverflow(uint16_t newFreq);
        void Restart();
        void Sweep();

        std::string GetDutyCycleParameterName();
        std::string GetFrequencyParameterName();
        std::string GetOutputParameterName();
        std::string GetEnveloppeOutputParameterName();

        int m_number;
        Tonic::Generator m_wave;
        SweepRegister m_sweepReg;
        WavePatternRegister m_waveReg;
        VolumeEnveloppeRegister m_volumeReg;
        uint8_t m_freqLsb = 0x00;
        FrequencyHighRegister m_freqMsbReg;

        uint16_t m_combinedFreq = 0x0000;
        double m_frequency = 0.0;
        uint8_t m_lengthCounter = 0;
        uint8_t m_sweepCounter = 0;
        uint8_t m_volumeCounter = 0;

        bool m_enabled = false;
        bool m_frequencyChanged = false;
        bool m_dutyChanged = false;
        bool m_enabledChanged = false;
        bool m_volumeChanged = false;

        size_t m_nbUpdateCalls = 0;
    };
}