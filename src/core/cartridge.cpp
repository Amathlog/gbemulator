#include <core/cartridge.h>
#include <algorithm>

using GBEmulator::Cartridge;

void Cartridge::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteContainer(m_externalRAM);
    visitor.WriteValue(m_currentExternalRAMBank);
    visitor.WriteValue(m_currentPrgDataBank);
}

void Cartridge::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadContainer(m_externalRAM);
    visitor.ReadValue(m_currentExternalRAMBank);
    visitor.ReadValue(m_currentPrgDataBank);
}

void Cartridge::Reset()
{
    std::fill(m_externalRAM.begin(), m_externalRAM.end(), 0x00);
    m_currentExternalRAMBank = 0;
    m_currentPrgDataBank = 1;
}

bool Cartridge::ReadByte(uint16_t addr, uint8_t& data)
{
    // ROM zone
    if (addr < 0x8000)
    {
        // Compute the bank we need. Between 0x0000 and 0x3FFF it is always 0
        // between 0x4000 and 0x7FFF it is the switchable one
        uint8_t prgDataBank = (addr & 0x4000) ? m_currentPrgDataBank : 0;
        // ROM Banks are 16kB in size.
        data = m_prgData[prgDataBank * 0x4000 + (addr & 0x3FFF)];
        return true;
    }
    // RAM zone
    else if (addr >= 0xA000 && addr < 0xC000)
    {
        // RAM banks are 8kB in size
        data = m_externalRAM[m_currentExternalRAMBank * 0x2000 + (addr & 0x1FFF)];
        return true;
    }

    return false;
}

bool Cartridge::WriteByte(uint16_t addr, uint8_t data)
{
    // We can only write to the RAM
    if (addr >= 0xA000 && addr < 0xC000)
    {
        // RAM banks are 8kB in size
        m_externalRAM[m_currentExternalRAMBank * 0x2000 + (addr & 0x1FFF)] = data;
        return true;
    }

    return false;
}