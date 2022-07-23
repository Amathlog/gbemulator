#pragma once

#include <exe/imguiWindows/imguiWindow.h>
#include <exe/window.h>
#include <exe/rendering/image.h>
#include <memory>
#include <string>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class TileDataWindow : public ImGuiWindow
    {
    public:
        TileDataWindow();

        WINDOW_ID_IMPL(AllWindowsId::TileDataWindowId);


    protected:
        void UpdateImage();
        void DrawInternal() override;
        const char* GetWindowName() const override { return "TileData##12"; }

        double m_lastUpdateTime;
        bool m_forceUpdate = false;

        std::vector<uint8_t> m_data;
        unsigned m_currentBlock = 0;
        std::unique_ptr<Image> m_image;
        unsigned m_texture;
    };
}