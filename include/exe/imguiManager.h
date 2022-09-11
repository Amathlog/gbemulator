#pragma once

#include "exe/imguiWindows/imguiWindow.h"
#include <string>
#include <exe/rendering/image.h>
#include <array>
#include <map>
#include <vector>


struct ImGuiContext;

namespace GBEmulatorExe
{
    class Window;

    struct Toggle
    {
        Toggle(bool _value) : value(_value) {}

        bool value = false;
        bool previous = false;
    };
    
    class ImguiManager
    {
    public:
        ImguiManager(Window* window);
        ~ImguiManager();

        bool ShouldClose() const { return m_closeRequested; }
        void ToggleMainMenu() { m_showMainMenu = !m_showMainMenu; }

        bool IsBreakOnStart() const { return m_breakOnStart.value; }

        void Update();

    private:
        void HandleFileExplorer();
        void HandlePerf(bool showFPS);
        void HandleBreakOnStart();

        void Serialize();
        void Deserialize();
        
        Window* m_window;
        ImGuiContext* m_context;

        bool m_showFileExplorer = false;
        bool m_closeRequested = false;
        bool m_showMainMenu = true;
        Toggle m_isSoundEnabled = false;
        Toggle m_breakOnStart = false;

        Format m_currentFormat = Format::ORIGINAL;

        std::array<bool, (size_t)Format::COUNT> m_changeFormats;
        
        inline static constexpr unsigned MAX_SAVE_STATES = 4;
        std::array<bool, MAX_SAVE_STATES> m_requestSaveState;
        std::array<bool, MAX_SAVE_STATES> m_requestLoadState;

        using ChildWidgetMap = std::map<int, std::unique_ptr<ImGuiWindow>>;
        ChildWidgetMap m_childWidgets;
    };
}