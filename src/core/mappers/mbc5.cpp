#include <core/mappers/mbc5.h>

using GBEmulator::MBC5;

bool MBC5::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr <= 0x1FFF)
    {
        // RAM enable
        m_ramEnabled = (data & 0x0F) == 0x0A;
        return true;
    }

    if (addr <= 0x2FFF)
    {
        // 8 lsb of second rom bank
        m_secondRomBank = (m_secondRomBank & 0x0100) | data;
        return true;
    }

    if (addr <= 0x3FFF)
    {
        // 9th bit of second rom bank
        m_secondRomBank = ((uint16_t)(data & 0x01) << 8) | (m_secondRomBank & 0x0F);
        return true;
    }

    if (addr <= 0x5FFF)
    {
        // RAM bank selection
        m_ramBank = data & 0x0F;
        return true;
    }

    return false;
}