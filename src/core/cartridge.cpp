#include <core/cartridge.h>
#include <algorithm>

using GBEmulator::Cartridge;

void Cartridge::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteContainer(m_externalRAM);
    visitor.WriteValue(m_currentExternalRAMBank);
}

void Cartridge::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadContainer(m_externalRAM);
    visitor.ReadValue(m_currentExternalRAMBank);
}

void Cartridge::Reset()
{
    std::fill(m_externalRAM.begin(), m_externalRAM.end(), 0x00);
    m_currentExternalRAMBank = 0;   
}