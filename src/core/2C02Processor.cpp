#include <core/2C02Processor.h>

using GBEmulator::Processor2C02;

uint8_t Processor2C02::ReadByte(uint16_t addr)
{
    uint8_t data = 0;
    if (addr == 0xFF40)
    {
        data = m_lcdRegister.flags;
    }
    else if (addr == 0xFF41)
    {
        data = m_lcdStatus.flags;
    }
    else if (addr == 0xFF42)
    {
        data = m_scrollY;
    }
    else if (addr == 0xFF43)
    {
        data = m_scrollX;
    }
    else if (addr == 0xFF44)
    {
        data = m_lY;
    }
    else if (addr == 0xFF45)
    {
        data = m_lYC;
    }
    else if (addr == 0xFF46)
    {
        // DMA transfer
        // TODO
    }
    else if (addr == 0xFF47)
    {
        data = m_gbBGPalette.flags;
    }
    else if (addr == 0xFF48)
    {
        data = m_gbOBJ0Palette.flags;
    }
    else if (addr == 0xFF49)
    {
        data = m_gbOBJ1alette.flags;
    }
    else if (addr == 0xFF4A)
    {
        data = m_wY;
    }
    else if (addr == 0xFF4B)
    {
        data = m_wX;
    }
    return data;
}

void Processor2C02::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr == 0xFF40)
    {
        m_lcdRegister.flags = data;
    }
    else if (addr == 0xFF41)
    {
        m_lcdStatus.flags = data;
    }
    else if (addr == 0xFF42)
    {
        m_scrollY = data;
    }
    else if (addr == 0xFF43)
    {
        m_scrollX = data;
    }
    else if (addr == 0xFF44)
    {
        // Read only
    }
    else if (addr == 0xFF45)
    {
        m_lYC = data;
    }
    else if (addr == 0xFF46)
    {
        // DMA transfer
        // TODO
    }
    else if (addr == 0xFF47)
    {
        m_gbBGPalette.flags = data;
    }
    else if (addr == 0xFF48)
    {
        m_gbOBJ0Palette.flags = data;
    }
    else if (addr == 0xFF49)
    {
        m_gbOBJ1alette.flags = data;
    }
    else if (addr == 0xFF4A)
    {
        m_wY = data;
    }
    else if (addr == 0xFF4B)
    {
        m_wX = data;
    }
}

void Processor2C02::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_lcdRegister.flags);
    visitor.WriteValue(m_lcdStatus.flags);
    visitor.WriteValue(m_scrollY);
    visitor.WriteValue(m_scrollX);
    visitor.WriteValue(m_lY);
    visitor.WriteValue(m_lYC);
    visitor.WriteValue(m_wY);
    visitor.WriteValue(m_wX);
    visitor.WriteValue(m_gbBGPalette);
    visitor.WriteValue(m_gbOBJ0Palette);
    visitor.WriteValue(m_gbOBJ1alette);
}

void Processor2C02::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_lcdRegister.flags);
    visitor.ReadValue(m_lcdStatus.flags);
    visitor.ReadValue(m_scrollY);
    visitor.ReadValue(m_scrollX);
    visitor.ReadValue(m_lY);
    visitor.ReadValue(m_lYC);
    visitor.ReadValue(m_wY);
    visitor.ReadValue(m_wX);
    visitor.ReadValue(m_gbBGPalette);
    visitor.ReadValue(m_gbOBJ0Palette);
    visitor.ReadValue(m_gbOBJ1alette);
}