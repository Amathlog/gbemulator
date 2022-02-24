#pragma once

#include <exe/window.h>
#include <memory>
#include <string>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class DebugWindow : public Window
    {
    public:
        DebugWindow(unsigned width, unsigned height);
        ~DebugWindow();

        void OnScreenResized(int width, int height) override { m_width = width; m_height = height; }

    protected:
        void InternalUpdate(bool externalSync) override;
        ImGuiContext* m_context;

        double m_lastUpdateTime;
        bool m_forceUpdate = false;
        bool m_isInBreakMode = false;
        int m_width;
        int m_height;

        uint16_t m_addressStart = 0x0000;
        std::vector<std::string> m_data;
    };
}