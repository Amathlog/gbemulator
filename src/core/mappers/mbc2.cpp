#include <core/mappers/mbc2.h>
#include <cassert>

using GBEmulator::MBC2;

MBC2::MBC2(const Header& header)
    : MapperBase(header)
{
    // Do some assertions
    assert(header.nbRomBanks <= 0x10 && "There are too many rom banks, exceeding MBC2 limitation");
    assert(header.nbRamBanks == 0x00 && "There are ram banks, there should not be.");
}

bool MBC2::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr <= 0x3FFF)
    {
        if (addr & 0x0100)
        {
            // ROM selection
            // Don't allow for second bank 0 (as it is already fixed in first ban)
            m_secondRomBank = data & 0x0F;
            if (m_secondRomBank == 0)
                m_secondRomBank = 1;
        }
        else
        {
            // RAM enabled
            m_ramEnabled = data == 0x0A;
        }
    }

    return false;
}