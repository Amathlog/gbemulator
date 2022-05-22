#include <core/2C02Processor.h>
#include <core/utils/utils.h>
#include <algorithm>

using GBEmulator::Processor2C02;
using GBEmulator::GBCPaletteData;
using GBEmulator::GBCPaletteAccess;

namespace // annonymous
{
    inline uint8_t ReadGBCPaletteData(const Processor2C02::GBCPaletteDataArray& palettesData, const GBCPaletteAccess& paletteAccess)
    {
        uint8_t paletteIndex = paletteAccess.address >> 3;
        uint8_t colorIndex = (paletteAccess.address >> 1) % 0x03;
        uint8_t isHigherBit = (paletteAccess.address & 0x01) > 0;

        uint16_t color = palettesData[paletteIndex].colors[colorIndex].data;

        return isHigherBit ? (color >> 8) : (color & 0x00FF);
    }

    inline void WriteGBCPaletteData(Processor2C02::GBCPaletteDataArray& palettesData, GBCPaletteAccess& paletteAccess, uint8_t data)
    {
        uint8_t paletteIndex = paletteAccess.address >> 3;
        uint8_t colorIndex = (paletteAccess.address >> 1) % 0x03;
        uint8_t isHigherBit = (paletteAccess.address & 0x01) > 0;

        uint16_t& color = palettesData[paletteIndex].colors[colorIndex].data;

        if (isHigherBit)
            color = (data << 8) | (color & 0x00FF);
        else
            color = (color & 0xFF00) | (data);

        if (paletteAccess.shouldIncr)
            paletteAccess.address = (paletteAccess.address + 1) % 0x3F;
    }

    inline uint32_t RGB555ToRGB888(GBEmulator::RGB555 inputColor)
    {
        // For each color, we add a left shift of 3 to transform
        // 0-31 range (5 bits) to 0-255 range (8 bits)
        return ((uint32_t)(inputColor.R) << 19)
            + ((uint32_t)(inputColor.G) << 11)
            + ((uint32_t)(inputColor.B) << 3);
    }
}

inline void GBCPaletteData::Reset()
{
    std::for_each(colors.begin(), colors.end(), [](auto& item) { item.data = 0x0000; });
}

inline void GBCPaletteData::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    std::for_each(colors.cbegin(), colors.cend(), [&visitor](const auto& item) { visitor.WriteValue(item.data); });
}

inline void GBCPaletteData::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    std::for_each(colors.begin(), colors.end(), [&visitor](auto& item) { visitor.ReadValue(item.data); });
}

Processor2C02::Processor2C02()
{
    m_screen.resize(GB_NB_PIXELS * 3);
}

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
        data = m_gbOBJ1Palette.flags;
    }
    else if (addr == 0xFF4A)
    {
        data = m_wY;
    }
    else if (addr == 0xFF4B)
    {
        data = m_wX;
    }
    else if (addr == 0xFF68)
    {
        // Write only to gbc GB palette access
    }
    else if (addr == 0xFF69)
    {
        data = ReadGBCPaletteData(m_gbcBGPalettes, m_gbcBGPaletteAccess);
    }
    else if (addr == 0xFF6A)
    {
        // Write only to gbc OBJ palette access
    }
    else if (addr == 0xFF6B)
    {
        data = ReadGBCPaletteData(m_gbcOBJPalettes, m_gbcOBJPaletteAccess);
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
        m_gbOBJ1Palette.flags = data;
    }
    else if (addr == 0xFF4A)
    {
        m_wY = data;
    }
    else if (addr == 0xFF4B)
    {
        m_wX = data;
    }
    else if (addr == 0xFF68)
    {
        m_gbcBGPaletteAccess.address = data & 0x3F;
        m_gbcBGPaletteAccess.shouldIncr = (data & 0x80) > 0;
    }
    else if (addr == 0xFF69)
    {
        WriteGBCPaletteData(m_gbcBGPalettes, m_gbcBGPaletteAccess, data);
    }
    else if (addr == 0xFF6A)
    {
        m_gbcOBJPaletteAccess.address = data & 0x3F;
        m_gbcOBJPaletteAccess.shouldIncr = (data & 0x80) > 0;
    }
    else if (addr == 0xFF6B)
    {
        WriteGBCPaletteData(m_gbcOBJPalettes, m_gbcOBJPaletteAccess, data);
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
    visitor.WriteValue(m_gbOBJ1Palette);

    std::for_each(m_gbcBGPalettes.begin(), m_gbcBGPalettes.end(), [&visitor](auto& item) { item.SerializeTo(visitor); });
    std::for_each(m_gbcOBJPalettes.begin(), m_gbcOBJPalettes.end(), [&visitor](auto& item) { item.SerializeTo(visitor); });

    visitor.WriteValue(m_gbcBGPaletteAccess.shouldIncr);
    visitor.WriteValue(m_gbcBGPaletteAccess.address);
    visitor.WriteValue(m_gbcOBJPaletteAccess.shouldIncr);
    visitor.WriteValue(m_gbcOBJPaletteAccess.address);

    // Need to copy here.
    auto bgFifoCopy = m_bgFifo;
    auto objFifoCopy = m_objFifo;

    visitor.WriteQueue(bgFifoCopy);
    visitor.WriteQueue(objFifoCopy);

    visitor.WriteValue(m_lineDots);
    visitor.WriteValue(m_scanlines);
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
    visitor.ReadValue(m_gbOBJ1Palette);

    std::for_each(m_gbcBGPalettes.begin(), m_gbcBGPalettes.end(), [&visitor](auto& item) { item.DeserializeFrom(visitor); });
    std::for_each(m_gbcOBJPalettes.begin(), m_gbcOBJPalettes.end(), [&visitor](auto& item) { item.DeserializeFrom(visitor); });

    visitor.ReadValue(m_gbcBGPaletteAccess.shouldIncr);
    visitor.ReadValue(m_gbcBGPaletteAccess.address);
    visitor.ReadValue(m_gbcOBJPaletteAccess.shouldIncr);
    visitor.ReadValue(m_gbcOBJPaletteAccess.address);

    visitor.ReadQueue(m_bgFifo);
    visitor.ReadQueue(m_objFifo);

    visitor.ReadValue(m_lineDots);
    visitor.ReadValue(m_scanlines);
}

void Processor2C02::Reset()
{
    m_lcdRegister.flags = 0x00;
    m_lcdStatus.flags = 0x00;
    m_scrollY = 0x00;
    m_scrollX = 0x00;
    m_lY = 0x00;
    m_lYC = 0x00;
    m_wX = 0x00;
    m_wY = 0x00;

    m_gbBGPalette.flags = 0x00;
    m_gbOBJ0Palette.flags = 0x00;
    m_gbOBJ1Palette.flags = 0x00;

    std::for_each(m_gbcBGPalettes.begin(), m_gbcBGPalettes.end(), [](auto& item) { item.Reset(); });
    std::for_each(m_gbcOBJPalettes.begin(), m_gbcOBJPalettes.end(), [](auto& item) { item.Reset(); });

    m_gbcBGPaletteAccess.Reset();
    m_gbcOBJPaletteAccess.Reset();

    Utils::ClearContainer(m_bgFifo);
    Utils::ClearContainer(m_objFifo);

    m_lineDots = 0;
    m_scanlines = 0;
    std::memset(m_screen.data(), 0, m_screen.size());
    m_isFrameComplete = false;
}

void Processor2C02::Clock()
{
    m_isFrameComplete = false;
    if (m_scanlines <= 143)
    {
        // Drawing mode
        // 0 - 80 = OAM scan (Mode 2)
        if (m_lineDots <= 80)
        {
            // TODO
        }
        else
        {
            // We can be in Drawing pixels (Mode 3) or Horizontal Blank (Mode 0)
            if (m_lcdStatus.mode == 3)
            {
                // Drawing pixels
                // Max number of dots = 289 (after the 80 from OAM scan)
                if (m_lineDots == 368)
                {
                    m_lcdStatus.mode = 0;
                }
            }
            else
            {
                // Horizontal Blank
            }
        }
    }
    else
    {
        // Vertical blank (Mode 1)
    }

    // Set screen pixel
    if (m_currentLinePixel < 160 && m_scanlines < 144)
    {
        unsigned index = m_scanlines * GB_INTERNAL_WIDTH + m_currentLinePixel;
        if ((float)rand() / RAND_MAX > 0.5)
        {
            m_screen[3 * index] = 255;
            m_screen[3 * index + 1] = 255;
            m_screen[3 * index + 2] = 255;
        }
        else
        {
            m_screen[3 * index] = 0;
            m_screen[3 * index + 1] = 0;
            m_screen[3 * index + 2] = 0;
        }
        m_currentLinePixel++;
    }

    ++m_lineDots;
    if (m_lineDots == 456)
    {
        ++m_scanlines;
        if (m_scanlines == 144)
        {
            // Vertical blank mode (mode 1)
            m_lcdStatus.mode = 1;
        }
        else if (m_scanlines == 154)
        {
            // Start a new frame, with OAM scan (mode 2)
            m_scanlines = 0;
            m_lcdStatus.mode = 2;
        }

        m_lineDots = 0;
    }
    else if (m_lineDots == 80)
    {
        // OAM scan is done, go to mode 3.
        // Also clear the FIFOs
        m_lcdStatus.mode = 3;
        Utils::ClearContainer(m_bgFifo);
        Utils::ClearContainer(m_objFifo);
        m_currentLinePixel = 0;
    }

    m_isFrameComplete = m_currentLinePixel == 160 && m_scanlines == 143;
}