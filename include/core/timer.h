#pragma once

#include <core/serializable.h>
#include <cstdint>

namespace GBEmulator
{
    class Timer : public ISerializable
    {
    public:
        Timer();

        uint8_t ReadByte(uint16_t addr, bool readOnly = false);
        void WriteByte(uint16_t addr, uint8_t data);
        void Reset();

        // Returns true if the timer counter overflows (needs to fire an interrupt)
        bool Clock();

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

    private:
        uint8_t m_divider;
        uint8_t m_timerCounter;
        uint8_t m_timerModulo;
        uint8_t m_timerControl;
        size_t m_timerControlValue;
        bool m_enabled;

        size_t m_nbClocks;
    };
}