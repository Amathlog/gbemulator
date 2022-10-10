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
        FindRomsWindowId,
        OAMWindowId,
        BGWindowId,

        Undefined = 0xFFFFFFFF
    };

    class ImGuiWindow
    {
    public:
        virtual ~ImGuiWindow() = default;

        void Draw();
        virtual int GetWindowId() const { return AllWindowsId::Undefined; }
        
        bool m_open = false;

    protected:
        // Will be within an ImGui window context
        virtual void DrawInternal() = 0;
        virtual const char* GetWindowName() const = 0;

        float m_width;
        float m_height;
    };
}