#pragma once

#include <core/serializable.h>
#include <core/constants.h>
#include <array>
#include <queue>

namespace GBEmulator
{
    union LCDRegister
    {
        struct
        {
            uint8_t enable : 1;
            uint8_t windowTileMapArea : 1;
            uint8_t windowEnable : 1;
            uint8_t BGAndWindowTileAreaData : 1;
            uint8_t bgTileMapArea : 1;
            uint8_t objSize : 1;
            uint8_t objEnable : 1;
            uint8_t BGAndWindowPriority : 1;
        };

        uint8_t flags = 0x00;
    };

    union LCDStatus
    {
        struct
        {
            uint8_t unused : 1;
            // Interrupt sources (IS)
            uint8_t lYcEqualLYIS : 1;
            uint8_t mode2OAMIS : 1;
            uint8_t mode1VBlankIS : 1;
            uint8_t mode0HBlankIS : 1;
            uint8_t lYcEqualLY : 1;
            uint8_t mode : 2;
        };

        uint8_t flags = 0x00;
    };

    union GBPaletteData
    {
        struct
        {
            uint8_t color3 : 2;
            uint8_t color2 : 2;
            uint8_t color1 : 2;
            uint8_t color0 : 2;
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

    class Processor2C02 : public ISerializable
    {
    public:
        uint8_t ReadByte(uint16_t addr);
        void WriteByte(uint16_t addr, uint8_t data);

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();
        void Clock();

        using GBCPaletteDataArray = std::array<GBCPaletteData, 8>;

    private:
        LCDRegister m_lcdRegister;
        LCDStatus m_lcdStatus;

        uint8_t m_scrollY;
        uint8_t m_scrollX;
        uint8_t m_lY;
        uint8_t m_lYC;
        uint8_t m_wY;
        uint8_t m_wX;

        GBPaletteData m_gbBGPalette;
        GBPaletteData m_gbOBJ0Palette;
        GBPaletteData m_gbOBJ1Palette;

        // GBC Spcific

        GBCPaletteDataArray m_gbcBGPalettes;
        GBCPaletteDataArray m_gbcOBJPalettes;
        GBCPaletteAccess m_gbcBGPaletteAccess;
        GBCPaletteAccess m_gbcOBJPaletteAccess;

        // FIFOs
        std::queue<RGB555> m_bgFifo;
        std::queue<RGB555> m_objFifo;

        // Counters
        unsigned m_lineDots;
        unsigned m_scanlines;
    };
}