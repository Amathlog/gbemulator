#pragma once

#include <core/mappers/mapperBase.h>
#include <cstddef>

namespace GBEmulator
{
struct ClockCounterRegisters
{
    uint8_t seconds = 0x00;
    uint8_t minutes = 0x00;
    uint8_t hours = 0x00;
    uint8_t lowerDayCounter = 0x00;

    union Extra
    {
        struct
        {
            uint8_t msbDayCounter : 1;
            uint8_t unused : 5;
            uint8_t halt : 1;
            uint8_t dayCounterCarry : 1;
        };

        uint8_t reg = 0x00;
    } extra;
};

class MBC3 : public MapperBase
{
public:
    MBC3(const Header& header);

    void SerializeRam(Utils::IWriteVisitor& visitor) const override;
    void DeserializeRam(Utils::IReadVisitor& visitor) override;

    void Reset() override;

    bool WriteByte(uint16_t addr, uint8_t data) override;
    bool ReadByte(uint16_t addr, uint8_t& data) const override;

    void TickSecond() override;

private:
    void WriteClockRegister(uint8_t data);
    const uint8_t ReadClockRegister() const;

    uint8_t m_currentClockRegister = 0xFF;
    bool m_rtcEnabled = false;
    ClockCounterRegisters m_clockRegisters;
    ClockCounterRegisters m_latchedClockRegisters;
    bool m_firstLatchedSequence = false;
    std::size_t m_nbCycles = 0;
};
} // namespace GBEmulator