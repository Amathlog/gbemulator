#include <core/2C02Processor.h>

using GBEmulator::Processor2C02;

uint8_t Processor2C02::ReadByte(uint16_t addr)
{
    uint8_t data = 0;
    if (addr == 0xFF40)
    {
        data = m_lcdRegister.flags;
    }

    return data;
}

void Processor2C02::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr == 0xFF40)
    {
        m_lcdRegister.flags = data;
    }
}

void Processor2C02::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_lcdRegister.flags);
}

void Processor2C02::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_lcdRegister.flags);
}