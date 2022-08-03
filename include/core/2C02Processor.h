#pragma once

#include <core/serializable.h>
#include <core/constants.h>
#include <array>
#include <queue>

namespace GBEmulator
{
    class Bus;

    enum class InteruptSource
    {
        HBlank,
        VBlank,
        OAM,
        LYC
    };

    union LCDRegister
    {
        struct
        {
            uint8_t BGAndWindowPriority : 1;
            uint8_t objEnable : 1;
            uint8_t objSize : 1;
            uint8_t bgTileMapArea : 1;
            uint8_t BGAndWindowTileAreaData : 1;
            uint8_t windowEnable : 1;
            uint8_t windowTileMapArea : 1;
            uint8_t enable : 1;
        };

        uint8_t flags = 0x00;
    };

    union LCDStatus
    {
        struct
        {
            uint8_t mode : 2;
            uint8_t lYcEqualLY : 1;
            // Interrupt sources (IS)
            uint8_t mode0HBlankIS : 1;
            uint8_t mode1VBlankIS : 1;
            uint8_t mode2OAMIS : 1;
            uint8_t lYcEqualLYIS : 1;
            uint8_t unused : 1;
        };

        uint8_t flags = 0x00;
    };

    union GBPaletteData
    {
        struct
        {
            uint8_t color0 : 2;
            uint8_t color1 : 2;
            uint8_t color2 : 2;
            uint8_t color3 : 2;
        };

        uint8_t flags = 0x00;
    };

    struct GBCPaletteData
    {
        std::array<RGB555, 4> colors;

        void Reset();

        void SerializeTo(Utils::IWriteVisitor& visitor) const;
        void DeserializeFrom(Utils::IReadVisitor& visitor);
    };

    struct GBCPaletteAccess
    {
        bool shouldIncr = false;
        uint8_t address = 0x00;

        void Reset() { shouldIncr = false; address = 0x00; }
    };

    struct PixelFIFO
    {
        uint8_t color = 0x00;
        uint8_t palette = 0x00;
        uint8_t spritePriority = 0x00;
        uint8_t bgPriority = 0x00;
    };

    class Processor2C02 : public ISerializable
    {
    public:
        Processor2C02();

        uint8_t ReadByte(uint16_t addr, bool readOnly = false);
        void WriteByte(uint16_t addr, uint8_t data);

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        constexpr unsigned GetHeight() const { return GB_INTERNAL_HEIGHT; }
        constexpr unsigned GetWidth() const { return GB_INTERNAL_WIDTH; }
        const auto& GetScreen() const { return m_screen; }

        bool IsFrameComplete() const { return m_isFrameComplete; }

        void ConnectBus(Bus* bus) { m_bus = bus; }

        void Reset();
        void Clock();

        using GBCPaletteDataArray = std::array<GBCPaletteData, 8>;

    private:

        void DebugRenderNoise();
        void DebugRenderTileIds();
        void RenderPixelFifos();
        void RenderDisabledLCD();
        void SetInteruptFlag(InteruptSource is);

        Bus* m_bus = nullptr;
        LCDRegister m_lcdRegister;
        LCDStatus m_lcdStatus;

        uint8_t m_scrollY = 0x00;
        uint8_t m_scrollX = 0x00;
        uint8_t m_lY = 0x00;
        uint8_t m_lYC = 0x00;
        uint8_t m_wY = 0x00;
        uint8_t m_wX = 0x00;

        GBPaletteData m_gbBGPalette;
        GBPaletteData m_gbOBJ0Palette;
        GBPaletteData m_gbOBJ1Palette;

        // GBC Spcific

        GBCPaletteDataArray m_gbcBGPalettes;
        GBCPaletteDataArray m_gbcOBJPalettes;
        GBCPaletteAccess m_gbcBGPaletteAccess;
        GBCPaletteAccess m_gbcOBJPaletteAccess;

        // FIFOs
        std::queue<PixelFIFO> m_bgFifo;
        std::queue<PixelFIFO> m_objFifo;

        std::array<PixelFIFO, 8> m_currentFetchedBGPixels;
        std::array<PixelFIFO, 8> m_currentFetchedOBJPixels;

        uint8_t m_currentStagePixelFetcher = 0x00;
        uint8_t m_XOffsetBGTile = 0x00;
        uint16_t m_BGWindowTileAddress = 0x0000;
        uint8_t m_initialBGXScroll = 0x00;
        bool m_isWindowRendering = false;

        // Counters
        unsigned m_lineDots = 0x00;
        unsigned m_scanlines = 0x00;
        unsigned m_currentLinePixel = 0x00;
        uint8_t m_currentX = 0x00;
        uint8_t m_currentNbPixelsToRender = 0x00;

        // Screen
        std::vector<uint8_t> m_screen;
        bool m_isFrameComplete = false;
        bool m_isDisabled = false;
    };
}