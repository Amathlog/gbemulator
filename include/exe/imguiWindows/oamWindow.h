#pragma once

#include <array>
#include <core/2C02Processor.h>
#include <exe/imguiWindows/imguiWindow.h>
#include <exe/rendering/image.h>
#include <exe/window.h>
#include <memory>
#include <string>

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
    GBEmulator::Processor2C02::GBCPaletteDataArray m_gbcBGPalettes;

    std::array<GBEmulator::OAMEntry, 40> m_oamEntries;
    std::array<std::unique_ptr<Image>, 40> m_sprites;
    bool m_isGBC = false;
};
} // namespace GBEmulatorExe