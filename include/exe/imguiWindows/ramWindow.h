#pragma once

#include "exe/imguiWindows/imguiWindow.h"
#include <exe/window.h>
#include <memory>
#include <string>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class RamWindow : public ImGuiWindow
    {
    public:
        RamWindow();

        WINDOW_ID_IMPL(AllWindowsId::RamWindowId);

    protected:
        void DrawInternal() override;
        const char* GetWindowName() const override { return "Ram##12"; }

        double m_lastUpdateTime;
        bool m_forceUpdate = false;

        uint16_t m_addressStart = 0x0000;
        std::vector<uint8_t> m_data;
    };
}