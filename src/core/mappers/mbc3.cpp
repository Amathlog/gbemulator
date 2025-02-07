#include "core/constants.h"
#include <cassert>
#include <core/mappers/mbc3.h>

using GBEmulator::ClockCounterRegisters;
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
    visitor.WriteValue(m_rtcEnabled);
}

void MBC3::DeserializeRam(Utils::IReadVisitor& visitor)
{
    MapperBase::DeserializeRam(visitor);

    visitor.ReadValue(m_currentClockRegister);
    visitor.ReadValue(m_clockRegisters);
    visitor.ReadValue(m_rtcEnabled);
}

void MBC3::Reset()
{
    MapperBase::Reset();
    m_currentClockRegister = 0xFF;
    m_clockRegisters = ClockCounterRegisters();
    m_latchedClockRegisters = ClockCounterRegisters();
    m_firstLatchedSequence = false;
    m_rtcEnabled = false;
    m_startAddrCustomRead = 0x0000;
    m_endAddrCustomRead = 0x0000;
    m_ramBank = 0;
    m_nbCycles = 0;
}

bool MBC3::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr <= 0x1FFF)
    {
        // RAM enable
        m_ramEnabled = !!(data & 0x0A);
        // std::cout << "Set RAM enable to " << (m_ramEnabled ? "true" : "false") << std::endl;
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
            m_rtcEnabled = false;
            // std::cout << "Set RAM bank to " << (int)m_ramBank << std::endl;
        }
        else if (data >= 0x08 && data <= 0x0C)
        {
            m_currentClockRegister = data - 0x08;
            m_rtcEnabled = true;
            // std::cout << "Set RTC register to " << (int)data << std::endl;
        }

        return true;
    }

    if (addr <= 0x7FFF)
    {
        // Latch clock data
        if (data == 0x00)
        {
            m_firstLatchedSequence = true;
        }
        else if (data == 0x01 && m_firstLatchedSequence)
        {
            // Latchf
            m_latchedClockRegisters = m_clockRegisters;
            m_firstLatchedSequence = false;
            // std::cout << "Latch RTC register " << std::endl;
        }
        else
        {
            m_firstLatchedSequence = false;
        }

        return true;
    }

    // If we are mapped to a clock register
    if (addr >= 0xA000 && addr < 0xC000 && m_rtcEnabled)
    {
        if (m_ramEnabled)
        {
            WriteClockRegister(data);
        }
        else
        {
            m_ramEnabled = m_ramEnabled;
        }

        return true;
    }

    return false;
}

bool MBC3::ReadByte(uint16_t addr, uint8_t& data) const
{
    if (addr >= 0xA000 && addr < 0xC000 && m_rtcEnabled)
    {
        if (m_ramEnabled)
        {
            data = ReadClockRegister();
        }
        else
        {
            data = 0xFF;
        }

        return true;
    }

    return false;
}

void GBEmulator::MBC3::TickSecond()
{
#if !PRECISE_TIME
    return;
#endif

    if (m_clockRegisters.extra.halt)
        return;

    if (++m_nbCycles < GBEmulator::CPU_SINGLE_SPEED_FREQ)
        return;

    m_nbCycles -= GBEmulator::CPU_SINGLE_SPEED_FREQ;

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

void MBC3::WriteClockRegister(uint8_t data)
{
    // std::cout << "Write RTC register " << (int)m_currentClockRegister << ": " << (int)data << std::endl;

    switch (m_currentClockRegister)
    {
    case 0:
        m_clockRegisters.seconds = data;
        break;
    case 1:
        m_clockRegisters.minutes = data;
        break;
    case 2:
        m_clockRegisters.hours = data;
        break;
    case 3:
        m_clockRegisters.lowerDayCounter = data;
        break;
    case 4:
        m_clockRegisters.extra.reg = data;
        break;
    default:
        break;
    }
}

const uint8_t MBC3::ReadClockRegister() const
{
    // std::cout << "Read RTC register " << (int)m_currentClockRegister << std::endl;

    switch (m_currentClockRegister)
    {
    case 0:
        return m_latchedClockRegisters.seconds & 0x3F;
    case 1:
        return m_latchedClockRegisters.minutes & 0x3F;
    case 2:
        return m_latchedClockRegisters.hours & 0x1F;
    case 3:
        return m_latchedClockRegisters.lowerDayCounter;
    case 4:
        return m_latchedClockRegisters.extra.reg & 0xC1;
    default:
        return 0xFF;
    }
}