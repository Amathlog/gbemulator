#include <core/2C02Processor.h>
#include <core/bus.h>
#include <core/utils/utils.h>
#include <core/utils/tile.h>
#include <core/constants.h>
#include <algorithm>
#include <cstring>

using GBEmulator::Processor2C02;
using GBEmulator::GBCPaletteData;
using GBEmulator::GBPaletteData;
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
    visitor.WriteValue(m_currentLinePixel);
    visitor.WriteValue(m_currentBGX);

    visitor.WriteContainer(m_currentFetchedBGPixels);
    visitor.WriteContainer(m_currentFetchedOBJPixels);
    visitor.WriteValue(m_currentStagePixelFetcher);
    visitor.WriteValue(m_XOffsetBGTile);
    visitor.WriteValue(m_lsbXScroll);
    visitor.WriteValue(m_BGTileAddress);
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
    visitor.ReadValue(m_currentLinePixel);
    visitor.ReadValue(m_currentBGX);

    visitor.ReadContainer(m_currentFetchedBGPixels);
    visitor.ReadContainer(m_currentFetchedOBJPixels);
    visitor.ReadValue(m_currentStagePixelFetcher);
    visitor.ReadValue(m_XOffsetBGTile);
    visitor.ReadValue(m_lsbXScroll);
    visitor.ReadValue(m_BGTileAddress);
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
    m_currentLinePixel = 0;
    m_currentBGX = 0;
    std::memset(m_screen.data(), 0, m_screen.size());
    m_isFrameComplete = false;

    m_currentStagePixelFetcher = 0;
    m_XOffsetBGTile = 0;
    m_lsbXScroll = 0;
    m_BGTileAddress = 0;
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
    if (m_bgFifo.empty() && m_objFifo.empty())
        return;

    PixelFIFO bgPixel;
    if (!m_bgFifo.empty())
    {
        bgPixel = m_bgFifo.front();
        m_bgFifo.pop();
    }

    PixelFIFO objPixel;
    if (!m_objFifo.empty())
    {
        objPixel = m_objFifo.front();
        m_objFifo.pop();
    }

    unsigned screenIndex = 3 * (m_scanlines * GB_INTERNAL_WIDTH + m_currentLinePixel);

    // We can render either the BG pixel, or the OBJ pixel
    // BG: If the OBJ color is 0 (transparent) or BG over OBJ flag is on
    const RGB555* pixelColor = nullptr;
    if (objPixel.color == 0 || objPixel.bgPriority == 1)
    {
        // Draw BG pixel
        if (m_bus->GetMode() == Mode::GB)
        {
            pixelColor = &DEFAULT_PALETTE[GetColorIndexFromGBPalette(m_gbBGPalette, bgPixel.color)];
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
            pixelColor = &DEFAULT_PALETTE[GetColorIndexFromGBPalette(objPixel.palette == 0 ? m_gbOBJ0Palette : m_gbOBJ1Palette, objPixel.color)];
        }
        else
        {
            pixelColor = &(m_gbcOBJPalettes[objPixel.palette].colors[objPixel.color]);
        }
    }

    Utils::RGB555ToRGB888(*pixelColor, m_screen[screenIndex], m_screen[screenIndex + 1], m_screen[screenIndex + 2]);
    m_currentLinePixel++;
}

void Processor2C02::Clock()
{
    // Update the status
    m_lcdStatus.lYcEqualLY = m_lY == m_lYC;

    m_isFrameComplete = false;
    if (m_scanlines <= 143)
    {
        if (m_lineDots == 0)
        {
            m_lsbXScroll = m_scrollX & 0x07;
        }

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

                // First step: BG
                switch (m_currentStagePixelFetcher)
                {
                // For the first 3 steps, it takes 2 dots for each,
                // so sleep for this ones.
                case 1: // Fall-through
                case 3: // Fall-through
                case 5:
                    m_currentStagePixelFetcher++;
                    break;
                // Get Tile
                case 0:
                {
                    // To get the tile, we need first to know our vertical/horizontal scrolling
                    // For horizontal scrolling, we only update the 5 msb bits of the scroll X register.
                    // the 3 lsb one are fixed during the whole scanline.
                    // If we exceed 256, it wraps around.
                    uint8_t currentY = m_scrollY + m_scanlines;
                    uint8_t scrollX = (m_scrollX & 0xF8) | m_lsbXScroll;
                    uint8_t currentX = scrollX + m_currentBGX;

                    // We can then compute the coordinate of the tile to fetch
                    uint16_t tileCoordinate = (currentY / 8) * 32 + (currentX / 8);
                    // If we are in a middle of a tile, because of X scroll, take it into account
                    m_XOffsetBGTile = currentX % 8;

                    // Then fetch the tile ID in the tile map
                    uint16_t address = m_lcdRegister.bgTileMapArea == 0 ? 0x9800 : 0x9C00;
                    address += tileCoordinate;
                    uint8_t tileId = m_bus->ReadByte(address);

                    // Finally, compute the address of the tile data to read from in the next stage
                    // If tileAreaData is 0, the starting address is 0x9000 and the tileId is a signed integer
                    // Otherwise, the starting address is 0x8000 and the tileId is an unsigned integer
                    m_BGTileAddress = m_lcdRegister.BGAndWindowTileAreaData == 0 ? 0x9000 : 0x8000;
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

                    m_BGTileAddress += realTileId * 16; // Each tile is 16 bytes
                    // And offset the address given the current line
                    uint8_t YOffset = currentY % 8;
                    m_BGTileAddress += YOffset * 2;

                    m_currentStagePixelFetcher++;
                    break;
                }
                // Get Tile data low
                case 2:
                {
                    uint8_t tileLsb = m_bus->ReadByte(m_BGTileAddress);
                    tileLsb <<= m_XOffsetBGTile;
                    for (auto i = 0; i < 8 - m_XOffsetBGTile; ++i)
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
                    uint8_t tileMsb = m_bus->ReadByte(m_BGTileAddress + 1);
                    tileMsb <<= m_XOffsetBGTile;
                    for (auto i = 0; i < 8 - m_XOffsetBGTile; ++i)
                    {
                        m_currentFetchedBGPixels[i].color |= (tileMsb & 0x80) >> 6;
                        tileMsb <<= 1;
                    }

                    m_currentStagePixelFetcher++;
                    break;
                }
                // Push when ready
                default:
                    if (m_bgFifo.empty())
                    {
                        for (auto i = 0; i < 8 - m_XOffsetBGTile; ++i)
                        {
                            m_bgFifo.push(m_currentFetchedBGPixels[i]);
                        }
                        m_currentStagePixelFetcher = 0;
                        m_currentBGX += 8 - m_XOffsetBGTile;
                    }
                    break;
                }

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
    // There are multiple modes
    enum class RenderMode
    {
        DEBUG_RANDOM_NOISE,
        DEBUG_TILE_ID,
        NORMAL
    };

    constexpr RenderMode currentMode = RenderMode::NORMAL;

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
            InterruptRegister ifRegister;
            ifRegister.flag = m_bus->ReadByte(IF_REG_ADDR);
            ifRegister.vBlank = 1;
            m_bus->WriteByte(IF_REG_ADDR, ifRegister.flag);
        }
        else if (m_scanlines == 154)
        {
            // Start a new frame, with OAM scan (mode 2)
            m_scanlines = 0;
            m_lcdStatus.mode = 2;
        }

        m_lineDots = 0;
        m_currentBGX = 0;
        m_lY = (uint8_t)m_scanlines;
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