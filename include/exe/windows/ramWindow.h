#pragma once

#include <exe/window.h>
#include <memory>
#include <string>

struct ImGuiContext;

namespace GBEmulatorExe
{
    class RamWindow : public Window
    {
    public:
        RamWindow(unsigned width, unsigned height);
        ~RamWindow();

        void OnScreenResized(int width, int height) override { m_width = width; m_height = height; }

    protected:
        void InternalUpdate(bool externalSync) override;
        ImGuiContext* m_context;

        double m_lastUpdateTime;
        bool m_forceUpdate = false;
        int m_width;
        int m_height;

        uint16_t m_addressStart = 0x0000;
        std::vector<uint8_t> m_data;
    };
}