#pragma once

#include <exe/imguiWindows/imguiWindow.h>
#include <exe/window.h>
#include <exe/rendering/image.h>
#include <core/2C02Processor.h>
#include <memory>
#include <string>
#include <array>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class OAMWindow : public ImGuiWindow
    {
    public:
        OAMWindow();

        WINDOW_ID_IMPL(AllWindowsId::OAMWindowId);

    protected:
        void UpdateImage();
        void DrawInternal() override;
        const char* GetWindowName() const override { return "OAM##12"; }

        void DrawGBPalette();
        void DrawGBCPalette();

        double m_lastUpdateTime;
        bool m_forceUpdate = false;

        GBEmulator::GBPaletteData m_gbOBJ0Palette;
        GBEmulator::GBPaletteData m_gbOBJ1Palette;
        GBEmulator::Processor2C02::GBCPaletteDataArray m_gbcOBJPalettes;

        std::array<GBEmulator::OAMEntry, 40> m_oamEntries;
        std::array<std::unique_ptr<Image>, 40> m_sprites;
        bool m_isGBC = false;
    };
}