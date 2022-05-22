#pragma once

#define WINDOW_ID_IMPL(id) static int GetStaticWindowId() { return id; } \
int GetWindowId() const override { return GetStaticWindowId(); }
                           

namespace GBEmulatorExe 
{
    enum AllWindowsId
    {
        DebugWindowId,
        RamWindowId,
        TileDataWindowId,

        Undefined = 0xFFFFFFFF
    };

    class ImGuiWindow
    {
    public:
        virtual ~ImGuiWindow() = default;

        virtual void Draw() = 0;
        virtual int GetWindowId() const { return AllWindowsId::Undefined; }

        bool m_open = false;
    };
}