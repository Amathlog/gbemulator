#pragma once

#include "exe/imguiWindows/imguiWindow.h"
#include "exe/messageService/messages/debugPayload.h"
#include <exe/window.h>
#include <memory>
#include <string>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class DebugWindow : public ImGuiWindow
    {
    public:
        DebugWindow();
        void Draw() override;

        WINDOW_ID_IMPL(AllWindowsId::DebugWindowId);

    protected:
        void UpdateBreakStatus();

        double m_lastUpdateTime;
        bool m_forceUpdate = false;
        bool m_isInBreakMode = false;
        float m_width;
        float m_height;

        std::vector<std::string> m_data;
        CPURegistersInfo m_cpuRegisterInfo;
    };
}