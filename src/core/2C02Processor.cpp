#include <core/2C02Processor.h>
#include <core/bus.h>
#include <core/utils/utils.h>
#include <core/utils/tile.h>
#include <core/constants.h>
#include <algorithm>
#include <cmath>
#include <cstring>

using GBEmulator::Processor2C02;
using GBEmulator::GBCPaletteData;
using GBEmulator::GBPaletteData;
using GBEmulator::GBCPaletteAccess;
using GBEmulator::InteruptSource;

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

    inline uint8_t GetColorIndexFromGBPalette(const GBPaletteData& palette, uint8_t index)
    {
        return (palette.flags >> (index << 1)) & 0x03;
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
    m_selectedOAM.reserve(10); // Max of 10 sprites selected on a single line
}

uint8_t Processor2C02::ReadByte(uint16_t addr, bool /*readOnly*/)
{
    uint8_t data = 0;
    if (addr >= 0xFE00 && addr <= 0xFE9F)
    {
        // OAM
        // Index is selected using the 6 msb of the address lower nibble.
        // It will be between 0 and 39 (40 entry in total)
        uint8_t index = (addr & 0x00FF) >> 2;
        OAMEntry& entry = m_OAM[index];

        // Then for each entry, there are 4 bytes, determined by the 2 lsb of the address.
        switch(addr & 0x0003)
        {
        case 0:
            data = entry.yPosition;
            break;
        case 1:
            data = entry.xPosition;
            break;
        case 2:
            data = entry.tileIndex;
            break;
        case 3:
        default:
            data = entry.attributes.flags;
            break;
        }
    }
    else if (addr == 0xFF40)
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
    if (addr >= 0xFE00 && addr <= 0xFE9F)
    {
        // OAM
        // Index is selected using the 6 msb of the address lower nibble.
        // It will be between 0 and 39 (40 entry in total)
        uint8_t index = (addr & 0x00FF) >> 2;
        OAMEntry& entry = m_OAM[index];

        // Then for each entry, there are 4 bytes, determined by the 2 lsb of the address.
        switch(addr & 0x0003)
        {
        case 0:
            entry.yPosition = data;
            break;
        case 1:
            entry.xPosition = data;
            break;
        case 2:
            entry.tileIndex = data;
            break;
        case 3:
        default:
            entry.attributes.flags = data;
            break;
        }
    }
    else if (addr == 0xFF40)
    {
        m_lcdRegister.flags = data;
        if (m_lcdRegister.enable == 0)
            m_isDisabled = true;
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

    m_bgFifo.SerializeTo(visitor);
    m_objFifo.SerializeTo(visitor);

    visitor.WriteValue(m_lineDots);
    visitor.WriteValue(m_scanlines);
    visitor.WriteValue(m_currentLinePixel);
    visitor.WriteValue(m_initialBGXScroll);
    visitor.WriteValue(m_currentX);

    visitor.WriteContainer(m_currentFetchedBGPixels);
    visitor.WriteContainer(m_currentFetchedOBJPixels);
    visitor.WriteValue(m_currentStagePixelFetcher);
    visitor.WriteValue(m_BGWindowTileAddress);
    visitor.WriteValue(m_isDisabled);
    visitor.WriteValue(m_currentNbPixelsToRender);

    visitor.WriteContainer(m_OAM);
    visitor.WriteContainer(m_selectedOAM);
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

    m_bgFifo.DeserializeFrom(visitor);
    m_objFifo.DeserializeFrom(visitor);

    visitor.ReadValue(m_lineDots);
    visitor.ReadValue(m_scanlines);
    visitor.ReadValue(m_currentLinePixel);
    visitor.ReadValue(m_initialBGXScroll);
    visitor.ReadValue(m_currentX);

    visitor.ReadContainer(m_currentFetchedBGPixels);
    visitor.ReadContainer(m_currentFetchedOBJPixels);
    visitor.ReadValue(m_currentStagePixelFetcher);
    visitor.ReadValue(m_BGWindowTileAddress);
    visitor.ReadValue(m_isDisabled);
    visitor.ReadValue(m_currentNbPixelsToRender);

    visitor.ReadContainer(m_OAM);
    visitor.ReadContainer(m_selectedOAM);
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

    m_bgFifo.Clear();
    m_objFifo.Clear();

    m_lineDots = 0;
    m_scanlines = 0;
    m_currentLinePixel = 0;
    m_currentX = 0;
    std::memset(m_screen.data(), 0, m_screen.size());
    m_isFrameComplete = false;

    m_currentStagePixelFetcher = 0;
    m_BGWindowTileAddress = 0;
    m_initialBGXScroll = 0;
    m_currentNbPixelsToRender = 0;

    m_OAM.fill(OAMEntry());
    m_selectedOAM.clear();
}

inline void Processor2C02::DebugRenderNoise()
{
    // Debug function to render noise on the screen, to test that
    // rendering is working

    unsigned index = m_scanlines * GB_INTERNAL_WIDTH + m_currentLinePixel;
    if ((float)rand() / (float)RAND_MAX > 0.5)
    {
        m_screen[3 * index] = (uint8_t)((float)rand() / (float)RAND_MAX * 255.0f);
        m_screen[3 * index + 1] = (uint8_t)((float)rand() / (float)RAND_MAX * 255.0f);
        m_screen[3 * index + 2] = (uint8_t)((float)rand() / (float)RAND_MAX * 255.0f);
    }
    else
    {
        m_screen[3 * index] = 0;
        m_screen[3 * index + 1] = 0;
        m_screen[3 * index + 2] = 0;
    }
}

inline void Processor2C02::DebugRenderTileIds()
{
    // Debug function to render the tile map ids
    // It maps all possible ids (256) to a gradient color
    // 0 -> 64:     Red -> Yellow
    // 64 -> 128:   Yellow -> Green
    // 128 -> 192:  Green -> Cyan
    // 192 -> 255:  Cyan -> Blue
    // 
    // Find the tile map index
    unsigned columnIndex = m_currentLinePixel / 5;
    unsigned rowIndex = m_scanlines / 4;

    uint16_t baseAddress = m_lcdRegister.bgTileMapArea == 0 ? 0x9800 : 0x9C00;
    uint16_t tileMapAddress = baseAddress + rowIndex * 32 + columnIndex;

    uint8_t data = m_bus->ReadByte(tileMapAddress);

    unsigned screenIndex = m_scanlines * GB_INTERNAL_WIDTH + m_currentLinePixel;

    uint8_t r, g, b;
    if (data < 64)
    {
        r = 255;
        g = data * 4;
        b = 0;
    }
    else if (data < 128)
    {
        r = 255 - (data - 64) * 4;
        g = 255;
        b = 0;
    }
    else if (data < 192)
    {
        r = 0;
        g = 255;
        b = (data - 128) * 4;
    }
    else
    {
        r = 0;
        g = 255 - (data - 192) * 4;
        b = 255;
    }

    m_screen[screenIndex * 3] = r;
    m_screen[screenIndex * 3 + 1] = g;
    m_screen[screenIndex * 3 + 2] = b;
}

inline void Processor2C02::RenderPixelFifos()
{
    if (m_bgFifo.Empty() && m_objFifo.Empty())
        return;

    PixelFIFO bgPixel;
    if (!m_bgFifo.Empty())
    {
        bgPixel = m_bgFifo.Pop();
    }

    PixelFIFO objPixel;
    if (!m_objFifo.Empty())
    {
        objPixel = m_objFifo.Pop();
    }

    unsigned screenIndex = 3 * (m_scanlines * GB_INTERNAL_WIDTH + m_currentLinePixel);

    const auto& gbPalette = GB_ORIGINAL_PALETTE;

    // We can render either the BG pixel, or the OBJ pixel
    // BG: If the OBJ color is 0 (transparent) or BG over OBJ flag is on
    const RGB555* pixelColor = nullptr;
    if (objPixel.color == 0 || (objPixel.bgPriority == 1 && bgPixel.color != 0))
    {
        // Draw BG pixel
        if (m_bus->GetMode() == Mode::GB)
        {
            pixelColor = &gbPalette[GetColorIndexFromGBPalette(m_gbBGPalette, bgPixel.color)];
        }
        else
        {
            pixelColor = &(m_gbcBGPalettes[bgPixel.palette].colors[bgPixel.color]);
        }
    }
    else
    {
        // Draw OBJ pixel
        if (m_bus->GetMode() == Mode::GB)
        {
            pixelColor = &gbPalette[GetColorIndexFromGBPalette(objPixel.palette == 0 ? m_gbOBJ0Palette : m_gbOBJ1Palette, objPixel.color)];
        }
        else
        {
            pixelColor = &(m_gbcOBJPalettes[objPixel.palette].colors[objPixel.color]);
        }
    }

    Utils::RGB555ToRGB888(*pixelColor, m_screen[screenIndex], m_screen[screenIndex + 1], m_screen[screenIndex + 2]);
    m_currentLinePixel++;
}

inline void Processor2C02::RenderDisabledLCD()
{
    // White screen
    std::memset(m_screen.data(), 0xFF, m_screen.size());
}

void Processor2C02::SetInteruptFlag(InteruptSource is)
{
    bool changed = false;
    InterruptRegister ifRegister;
    ifRegister.flag = m_bus->ReadByte(IF_REG_ADDR); 

    switch (is)
    {
    case InteruptSource::VBlank:
        ifRegister.vBlank = 1;
        if (m_lcdStatus.mode1VBlankIS)
            ifRegister.lcdStat = 1;
        changed = true;
        break;
    
    case InteruptSource::HBlank:
        if (m_lcdStatus.mode0HBlankIS)
        {
            ifRegister.lcdStat = 1;
            changed = true;
        }
        break;

    case InteruptSource::OAM:
        if (m_lcdStatus.mode2OAMIS)
        {
            ifRegister.lcdStat = 1;
            changed = true;
        }
        break;

    case InteruptSource::LYC:
        if (m_lcdStatus.lYcEqualLYIS)
        {
            ifRegister.lcdStat = 1;
            changed = true;
        }
        break;
    default:
        break;
    }

    if (changed)
        m_bus->WriteByte(IF_REG_ADDR, ifRegister.flag);
}

// Only support BG and Window
void Processor2C02::OriginalPixelFetcher()
{
    // First step: BG
    switch (m_currentStagePixelFetcher)
    {
    // For the first 3 steps, it takes 2 dots for each,
    // so sleep for this ones.
    // The fourth step is also sleep, for 2 dots (case 6 and 7)
    case 1: // Fall-through
    case 3: // Fall-through
    case 5: // Fall-through
    case 6: // Fall-through
    case 7: // Fall-through
        m_currentStagePixelFetcher++;
        break;
    // Get Tile
    case 0:
    {
        // For the tile, we need to know if we have to render a BG tile or a Window tile.
        // We need to render a window tile, if the current pixel X and Y are greater than the 
        // window scroll X and Y register and window is enabled.
        // From this moment we only render window tiles, for the rest of the line.
        // But before that we need to render BG tiles. We render BG tiles until we reach the
        // window tile. It could mean that we have to render not a complete BG tile.
        // For example, if WX is 10, we will have a first BG tile complete (8 pixels) and a second
        // BG tile with 2 pixels (so 25% of the tile). And then the window.

        // So first, check if we have to render a window tile
        // By default, we will try to render 8 pixels.
        m_currentNbPixelsToRender = 0;
        uint8_t maxNbBGPixels = 8;
        m_isWindowRendering = false;

        // We need to take into account that wX range is [0, 166], therefore
        // values of wX between 0 and 7 means that the full screen will be covered by the window
        uint8_t realWX = m_wX < 7 ? 0 : m_wX - 7;

        if (m_lcdRegister.windowEnable)
        {
            // Check Y
            if (m_scanlines > m_wY)
            {
                // Check X
                if (m_currentX >= realWX)
                {
                    // In this case we are totally into the window, no BG to draw
                    maxNbBGPixels = 0;
                    m_isWindowRendering = true;
                }
                else
                {
                    if (m_currentX + 8 > realWX)
                    {
                        // In this case, we have an overlap, so we need to reduce the number of BG pixels to draw
                        maxNbBGPixels = realWX - m_currentX;
                    }
                }
            }
        }

        auto fetchTileAddress = [this](uint8_t X, uint8_t Y, uint8_t maxPixelsToRender)
        {
            // Compute the address of the BG tile using the lcd control register to
            // know where the tile map is in memory.
            uint16_t tileCoordinate = (Y / 8) * 32 + (X / 8);
            uint8_t tileMapAreaRegister = m_isWindowRendering ? m_lcdRegister.windowTileMapArea : m_lcdRegister.bgTileMapArea;
            uint16_t tileAddress = tileMapAreaRegister == 0 ? 0x9800 : 0x9C00;
            tileAddress += tileCoordinate;

            // If we are in a middle of a tile, because of X scroll, take it into account
            // We also cannot draw more than the specified number of pixels to draw.
            m_currentNbPixelsToRender = std::min<uint8_t>(maxPixelsToRender, 8 - (X % 8));

            uint8_t tileId = m_bus->ReadByte(tileAddress);

            // Finally, compute the address of the tile data to read from in the next stage
            // If tileAreaData is 0, the starting address is 0x9000 and the tileId is a signed integer
            // Otherwise, the starting address is 0x8000 and the tileId is an unsigned integer
            m_BGWindowTileAddress = m_lcdRegister.BGAndWindowTileAreaData == 0 ? 0x9000 : 0x8000;
            int16_t realTileId = 0x0000;
            if (m_lcdRegister.BGAndWindowTileAreaData == 0)
            {
                int8_t temp = (int8_t)(tileId);
                realTileId = temp;
            }
            else
            {
                realTileId = tileId;
            }

            m_BGWindowTileAddress += realTileId * 16; // Each tile is 16 bytes
            // And offset the address given the current line
            uint8_t YOffset = Y % 8;
            m_BGWindowTileAddress += YOffset * 2;
        };

        // From there, we either try to render window or BG
        if (m_isWindowRendering)
        {
            uint8_t Y = m_scanlines - m_wY;
            // For X, we need to be a bit more careful, because m_wX range is [0, 166]
            // so we need to offset it.
            uint8_t X = m_currentX - realWX;
            fetchTileAddress(X, Y, 8);
        }
        else
        {
            // To get the tile, we need first to know our vertical/horizontal scrolling
            // For horizontal scrolling, we only update the 5 msb bits of the scroll X register.
            // the 3 lsb one are fixed during the whole scanline.
            // If we exceed 256, it wraps around.
            uint8_t currentBGY = m_scrollY + m_scanlines;
            uint8_t scrollBGX = (m_scrollX & 0xF8) + m_initialBGXScroll;
            uint8_t currentBGX = scrollBGX + m_currentX;

            fetchTileAddress(currentBGX, currentBGY, maxNbBGPixels);
        }

        m_currentStagePixelFetcher++;
        break;
    }
    // Get Tile data low
    case 2:
    {
        uint8_t tileLsb = m_bus->ReadByte(m_BGWindowTileAddress);
        tileLsb <<= (8 - m_currentNbPixelsToRender);
        for (auto i = 0; i < m_currentNbPixelsToRender; ++i)
        {
            m_currentFetchedBGPixels[i].color = (tileLsb & 0x80) >> 7;
            tileLsb <<= 1;
        }

        m_currentStagePixelFetcher++;
        break;
    }
    // Get Tile data high
    case 4:
    {
        uint8_t tileMsb = m_bus->ReadByte(m_BGWindowTileAddress + 1);
        tileMsb <<= (8 - m_currentNbPixelsToRender);
        for (auto i = 0; i < m_currentNbPixelsToRender; ++i)
        {
            m_currentFetchedBGPixels[i].color |= (tileMsb & 0x80) >> 6;
            tileMsb <<= 1;
        }

        m_currentStagePixelFetcher++;
        break;
    }
    // Push when ready
    default:
        if (m_bgFifo.Empty())
        {
            for (auto i = 0; i < m_currentNbPixelsToRender; ++i)
            {
                m_bgFifo.Push(m_currentFetchedBGPixels[i]);
            }
            m_currentStagePixelFetcher = 0;
            m_currentX += m_currentNbPixelsToRender;
        }
        break;
    }
}

void Processor2C02::SimplifiedPixelFetcher()
{
    // In this version, we "hack" our way by pushing all the pixels on one dot.

    auto fetchTileAddress = [this](uint8_t X, uint8_t Y, bool isWindow)
    {
        // Compute the address of the BG tile using the lcd control register to
        // know where the tile map is in memory.
        uint16_t tileCoordinate = (Y / 8) * 32 + (X / 8);
        uint8_t tileMapAreaRegister = isWindow ? m_lcdRegister.windowTileMapArea : m_lcdRegister.bgTileMapArea;
        uint16_t tileAddress = tileMapAreaRegister == 0 ? 0x9800 : 0x9C00;
        tileAddress += tileCoordinate;

        uint8_t tileId = m_bus->ReadByte(tileAddress);

        // Finally, compute the address of the tile data to read from in the next stage
        // If tileAreaData is 0, the starting address is 0x9000 and the tileId is a signed integer
        // Otherwise, the starting address is 0x8000 and the tileId is an unsigned integer
        uint16_t addr = m_lcdRegister.BGAndWindowTileAreaData == 0 ? 0x9000 : 0x8000;
        int16_t realTileId = 0x0000;
        if (m_lcdRegister.BGAndWindowTileAreaData == 0)
        {
            int8_t temp = (int8_t)(tileId);
            realTileId = temp;
        }
        else
        {
            realTileId = tileId;
        }

        addr += realTileId * 16; // Each tile is 16 bytes
        // And offset the address given the current line
        uint8_t YOffset = Y % 8;
        addr += YOffset * 2;

        return addr;
    };

    const bool isGB = m_bus->GetMode() == Mode::GB;

    auto pixelFetch = [this, isGB](uint16_t addr, auto& pixelArray, uint8_t startIndex, 
                                    uint8_t endIndex, bool xFlip, const OAMEntry* oamEntry = nullptr)
    {
        uint8_t tileLsb = m_bus->ReadByte(addr);
        uint8_t tileMsb = m_bus->ReadByte(addr + 1);

        auto reverseByte = [](uint8_t b) -> uint8_t
        {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
        };

        // In case of xFilp, reverse the bits
        if (xFlip)
        {
            tileLsb = reverseByte(tileLsb);
            tileMsb = reverseByte(tileMsb);
        }

        uint8_t nbPixelsToRender = endIndex - startIndex;
        tileLsb <<= (8 - nbPixelsToRender);
        tileMsb <<= (8 - nbPixelsToRender);
        for (auto i = 0; i < nbPixelsToRender; ++i)
        {
            uint8_t color = ((tileMsb & 0x80) >> 6) | ((tileLsb & 0x80) >> 7);
            if (color != 0)
                pixelArray[startIndex + i].color = color;

            tileMsb <<= 1;
            tileLsb <<= 1;

            if (oamEntry != nullptr && color != 0)
            {
                pixelArray[startIndex + i].bgPriority = oamEntry->attributes.bgAndWindowOverObj;
                pixelArray[startIndex + i].palette = isGB ? oamEntry->attributes.paletteNumberGB : oamEntry->attributes.paletteNumberGBC;
            }
        }
    };
    
    // First fetch all the pixel colors for the BG
    uint8_t yBG = m_scanlines + m_scrollY;
    // There can be at most 167 pixels fetched (fetch more even if we don't use them)
    std::array<PixelFIFO, 167> bgPixels;
    for (uint8_t x = 0; x < 160;)
    {
        uint8_t realX = x + (m_scrollX & 0xF8) + m_initialBGXScroll;
        uint8_t startX = x;

        if (x == 0 && m_initialBGXScroll != 0)
        {
            x += 8 - m_initialBGXScroll;
        }
        else
        {
            x += 8;
        }

        uint16_t tileAddr = fetchTileAddress(realX, yBG, false);
        pixelFetch(tileAddr, bgPixels, startX, x, false);
    }

    // Do the same for the window, only if it is enabled
    uint8_t yWindow = m_scanlines - m_wY;
    uint8_t nbWindowTiles = (uint8_t)std::ceil((166 - m_wX) / 8.f);
    std::array<PixelFIFO, 167> windowPixels;
    bool shouldDrawWindow = m_lcdRegister.windowEnable && (m_scanlines >= m_wY);
    if (shouldDrawWindow)
    {
        uint8_t xWindow = 0;
        for (uint8_t i = 0; i < nbWindowTiles; ++i)
        {
            uint16_t tileAddr = fetchTileAddress(xWindow, yWindow, true);

            uint8_t endX = (i == 0 && m_wX < 7) ? 7 - m_wX : xWindow + 8;

            pixelFetch(tileAddr, windowPixels, xWindow, endX, false);
            xWindow = endX;
        }
    }

    // And merge it into the pixel FIFO
    for (uint8_t i = 0; i < 160; ++i)
    {
        if (shouldDrawWindow && i + 7 >= m_wX)
        {
            // Window pixel
            m_bgFifo.Push(windowPixels[i + 7 - m_wX]);
        }
        else
        {   
            // BG pixels
            m_bgFifo.Push(bgPixels[i]);
        }
    }

    // Then redo the same thing for the sprites
    // Do it in reverse, for the highest priority sprite to override the lowest one
    std::array<PixelFIFO, 167> spritePixels;
    bool shouldDrawObj = m_lcdRegister.objEnable && !m_selectedOAM.empty();
    if (shouldDrawObj)
    {
        for (auto it = m_selectedOAM.rbegin(); it != m_selectedOAM.rend(); ++it)
        {
            const OAMEntry& entry = m_OAM[*it];

            // A position of 0 or more than 168 is hidden
            if (entry.xPosition == 0 || entry.xPosition >= 168)
                continue;

            uint16_t tileAddress = 0x8000;
            uint8_t objSize = m_lcdRegister.objSize == 0 ? 8 : 16;
            uint8_t yOffset = m_scanlines + 16 - entry.yPosition;
            if (entry.attributes.yFlip)
            {
                // In case of yFlip, we need to reverse the offset
                yOffset = objSize - yOffset;
            }

            if (m_lcdRegister.objSize == 0)
            {
                // 8x8 sprites
                tileAddress += (entry.tileIndex * 16) + yOffset * 2;
            }
            else
            {
                // 8x16 sprites
                tileAddress += ((entry.tileIndex & 0xFE) * 16);
                if (yOffset >= 8)
                {
                    tileAddress += 16 + (yOffset - 8) * 2;
                }
                else
                {
                    tileAddress += yOffset * 2;
                }
            }

            uint8_t startX = entry.xPosition < 8 ? 0 : entry.xPosition - 8;
            uint8_t endX = entry.xPosition;
            pixelFetch(tileAddress, spritePixels, startX, endX, entry.attributes.xFlip, &entry);
        }
    }

    // And push all to the pixel FIFO
    if (shouldDrawObj)
    {
        for (uint8_t i = 0; i < 160; ++i)
            m_objFifo.Push(spritePixels[i]);
    }
}

void Processor2C02::Clock()
{
    // Update the status
    m_lcdStatus.lYcEqualLY = m_lY == m_lYC;
    if (m_lcdStatus.lYcEqualLY)
        SetInteruptFlag(InteruptSource::LYC);

    m_isFrameComplete = false;
    if (m_scanlines <= 143)
    {
        if (m_lineDots == 0)
        {
            // The 3 lsb of the scrollX register are fixed for the scanline
            m_initialBGXScroll = m_scrollX & 0x07;
        }

        // Drawing mode
        // 0 - 79 = OAM scan (Mode 2)
        if (m_lineDots < 80)
        {
            // Only do stuff on even numbers and if the OBJ are enabled and if we didn't reach the limit of 10 selected sprites
            if (m_lineDots % 2 == 0 && m_lcdRegister.objEnable == 1 && m_selectedOAM.size() < 10)
            {
                // Get the current entry
                uint8_t index = m_lineDots >> 1;
                const OAMEntry& entry = m_OAM[index];

                uint8_t objSize = m_lcdRegister.objSize == 0 ? 8 : 16;

                // A sprite is selected if the current scanline (+ 16) is between the y position and the y position + its size (8 or 16)
                if (m_scanlines + 16 >= entry.yPosition && m_scanlines + 16 < entry.yPosition + objSize)
                {
                    m_selectedOAM.push_back(index);
                }
            }

            // On GB, OAM priority is given their X position. On GBC it's their order in the OAM
            // So on GB, sort the vector accroding to their X position
            if (m_lineDots == 79 && !m_selectedOAM.empty() && m_bus->GetMode() == Mode::GB)
            {
                std::sort(m_selectedOAM.begin(), m_selectedOAM.end(), [this](uint8_t a, uint8_t b) -> bool
                {
                    return m_OAM[a].xPosition < m_OAM[b].xPosition;
                });
            }
        }
        else
        {
            // We can be in Drawing pixels (Mode 3) or Horizontal Blank (Mode 0)
            if (m_lcdStatus.mode == 3)
            {
                // Drawing pixels
                constexpr bool useSimplefied = true;
                if constexpr (useSimplefied)
                {
                    if (m_lineDots == 80)
                        SimplifiedPixelFetcher();
                }
                else
                {
                    OriginalPixelFetcher();
                }

                // Max number of dots = 289 (after the 80 from OAM scan)
                if (m_lineDots == 368)
                {
                    m_lcdStatus.mode = 0;
                    SetInteruptFlag(InteruptSource::HBlank);
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
    // There are multiple modes
    enum class RenderMode
    {
        DEBUG_RANDOM_NOISE,
        DEBUG_TILE_ID,
        NORMAL,
        DISABLED
    };

    RenderMode currentMode = m_isDisabled ? RenderMode::DISABLED : RenderMode::NORMAL;

    if (m_currentLinePixel < 160 && m_scanlines < 144)
    {
        switch (currentMode)
        {
        case RenderMode::DEBUG_RANDOM_NOISE:
            DebugRenderNoise();
            m_currentLinePixel++;
            break;
        case RenderMode::DEBUG_TILE_ID:
            DebugRenderTileIds();
            m_currentLinePixel++;
            break;
        case RenderMode::NORMAL:
            // m_currentLinePixel will be incremented if a pixel was emitted from the FIFO
            // Therefore let this method handle the increment, if needed.
            RenderPixelFifos();
            break;
        case RenderMode::DISABLED:
            RenderDisabledLCD();
            m_currentLinePixel++;
            break;
        }
    }

    ++m_lineDots;
    if (m_lineDots == 456)
    {
        ++m_scanlines;
        if (m_scanlines == 144)
        {
            // Vertical blank mode (mode 1)
            m_lcdStatus.mode = 1;

            // Set the IF register bit for VBlank to 1
            SetInteruptFlag(InteruptSource::VBlank);
        }
        else if (m_scanlines == 154)
        {
            // Start a new frame, with OAM scan (mode 2)
            m_scanlines = 0;

            // Re-enable the LCD at a start of a new frame, if it is enabled.
            m_isDisabled = m_lcdRegister.enable == 0;
        }

        if (m_scanlines >= 0 && m_scanlines < 144)
        {
            // OAM scan
            m_lcdStatus.mode = 2;
            SetInteruptFlag(InteruptSource::OAM);
            m_selectedOAM.clear();
        }

        m_lineDots = 0;
        m_currentX = 0;

        m_lY = (uint8_t)m_scanlines;
        m_currentStagePixelFetcher = 0;
    }
    else if (m_lineDots == 80)
    {
        // OAM scan is done, go to mode 3.
        // Also clear the FIFOs
        m_lcdStatus.mode = 3;
        m_bgFifo.Clear();
        m_objFifo.Clear();
        m_currentLinePixel = 0;
    }

    m_isFrameComplete = m_currentLinePixel == 160 && m_scanlines == 143;
}