#include <cassert>
#include <core/mappers/mbc3.h>

using GBEmulator::MBC3;

MBC3::MBC3(const Header& header)
    : MapperBase(header)
{
    // Do some assertions
    assert(header.nbRomBanks <= 0x80 && "There are too many rom banks, exceeding MBC3 limitation");
    assert(header.nbRamBanks <= 0x04 && "There are too many ram banks, exceeding MBC3 limitation");
}

void MBC3::SerializeRam(Utils::IWriteVisitor& visitor) const
{
    MapperBase::SerializeRam(visitor);

    visitor.WriteValue(m_currentClockRegister);
    visitor.WriteValue(m_clockRegisters);
}

void MBC3::DeserializeRam(Utils::IReadVisitor& visitor)
{
    MapperBase::DeserializeRam(visitor);

    visitor.ReadValue(m_currentClockRegister);
    visitor.ReadValue(m_clockRegisters);
}

void MBC3::Reset()
{
    MapperBase::Reset();
    m_currentClockRegister = 0xFF;
}

bool MBC3::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr <= 0x1FFF)
    {
        // RAM enable
        m_ramEnabled = !!(data & 0x0A) && m_header.nbRamBanks > 0;
        return true;
    }

    if (addr <= 0x3FFF)
    {
        data &= 0x7F;
        // ROM bank number
        m_secondRomBank = data == 0x00 ? 0x01 : data;

        return true;
    }

    if (addr <= 0x5FFF)
    {
        // RAM bank selection or clock registers
        // Depends if we have enough RAM. If not, do nothing.
        if (data <= 0x03) // 32 kB
        {
            m_ramBank = data & 0x03;
            // No more custom read if we map a ram bank
            m_startAddrCustomRead = 0x0000;
            m_endAddrCustomRead = 0x0000;
        }
        else if (data >= 0x08 && data <= 0x0C)
        {
            m_currentClockRegister = data - 0x08;
            // Custom read for the RAM address range
            m_startAddrCustomRead = 0xA000;
            m_endAddrCustomRead = 0xC000;
        }

        return true;
    }

    if (addr <= 0x7FFF)
    {
        // Latch clock data
        // TODO

        return true;
    }

    // If we are mapped to a clock register
    if (m_currentClockRegister != 0xFF && addr >= 0xA000 && addr < 0xC000)
    {
        *GetClockRegister() = data;
    }

    return false;
}

bool MBC3::ReadByte(uint16_t addr, uint8_t& data) const
{
    if (m_currentClockRegister == 0xFF)
        return false;

    data = *GetClockRegister();

    return true;
}

void GBEmulator::MBC3::TickSecond() 
{
    m_clockRegisters.seconds++;
    if (m_clockRegisters.seconds >= 60)
    {
        m_clockRegisters.seconds = 0;
        m_clockRegisters.minutes++;
        if (m_clockRegisters.minutes >= 60)
        {
            m_clockRegisters.minutes = 0;
            m_clockRegisters.hours++;
            if (m_clockRegisters.hours >= 24)
            {
                m_clockRegisters.hours = 0;
                m_clockRegisters.lowerDayCounter++;
                if (m_clockRegisters.lowerDayCounter == 0x00)
                {
                    m_clockRegisters.lowerDayCounter = 0;
                    if (m_clockRegisters.extra.msbDayCounter == 0x01)
                    {
                        m_clockRegisters.extra.msbDayCounter = 0;
                        m_clockRegisters.extra.dayCounterCarry = 1;
                    }
                    else
                    {
                        m_clockRegisters.extra.msbDayCounter = 1;
                    }
                }
            }
        }
    }
}

uint8_t* MBC3::GetClockRegister()
{
    if (m_currentClockRegister == 0xFF)
        return nullptr;

    switch (m_currentClockRegister)
    {
    case 0:
        return &m_clockRegisters.seconds;
    case 1:
        return &m_clockRegisters.minutes;
    case 2:
        return &m_clockRegisters.hours;
    case 3:
        return &m_clockRegisters.lowerDayCounter;
    case 4:
        return &m_clockRegisters.extra.reg;
    }

    return nullptr;
}
