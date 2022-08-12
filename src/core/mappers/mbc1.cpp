#include <core/mappers/mbc1.h>
#include <cassert>

using GBEmulator::MBC1;

MBC1::MBC1(const Header& header)
    : MapperBase(header)
{
    // Do some assertions
    assert(header.nbRomBanks <= 0x80 && "There are too many rom banks, exceeding MBC1 limitation");
    assert(header.nbRamBanks <= 0x04 && "There are too many ram banks, exceeding MBC1 limitation");
}

void MBC1::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    MapperBase::SerializeTo(visitor);
    visitor.WriteValue(m_advancedBankingMode);
}
void MBC1::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    MapperBase::DeserializeFrom(visitor);
    visitor.ReadValue(m_advancedBankingMode);
}

void MBC1::Reset()
{
    MapperBase::Reset();
    m_advancedBankingMode = false;
}

bool MBC1::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr <= 0x1FFF)
    {
        // RAM enable
        m_ramEnabled = !!(data & 0x0A);
        return true;
    }

    if (addr <= 0x3FFF)
    {
        data &= 0x1F;
        // ROM bank number
        if (data == 0x00)
        {
            // Data = 0, means we want to select bank 0. But MBC1 will change it to 0x01
            m_secondRomBank = 0x0001;
        }
        else
        {
            // Otherwise, mask the data by the number of available rom banks
            // Keep the 2 highest bits of the bank. Will need to be set elsewhere (see below)
            m_secondRomBank = (m_secondRomBank & 0x0060) + (data & (uint8_t)(m_header.nbRomBanks - 1));

            // Note that in advanced mode, the 5th bit is ignored
            if (m_advancedBankingMode)
            {
                m_secondRomBank &= 0xEF;
            }
        }

        return true;
    }

    if (addr <= 0x5FFF)
    {
        // RAM bank selection or uppers bits of ROM bank selection
        // Depends if we have enough RAM or enough ROM. If not, do nothing.
        // Special behavior if we have advanced banking mode.
        if (m_header.nbRamBanks == 0x03) // 32 kB
        {
            m_ramBank = data & 0x03;
        }
        else if (m_header.nbRomBanks > 0x20)
        {
            // In advanced mode, we set the first rom bank with this register, the bits 5 and 4
            if (m_advancedBankingMode)
            {
                m_firstRomBank = (data & 0x03) << 4;
            }
            else
            {
                m_secondRomBank = ((data & 0x03) << 5) | (m_secondRomBank & 0x1F);
            }
        }

        return true;
    }

    if (addr <= 0x7FFF)
    {
        // Advanced banking mode. Disabled if the nb of rom banks is too low
        if (m_header.nbRomBanks > 0x20)
        {
            m_advancedBankingMode = !!(data & 0x01);
        }

        return true;
    }

    return false;
}