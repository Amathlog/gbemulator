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
    class WindowBase;

    struct Toggle
    {
        Toggle(bool _value) : value(_value) {}

        bool value = false;
        bool previous = false;
    };
    
    class ImguiManager
    {
    public:
        ImguiManager(WindowBase* window);
        ~ImguiManager();

        bool ShouldClose() const { return m_closeRequested; }
        void ToggleMainMenu() { m_showMainMenu = !m_showMainMenu; }

        bool IsBreakOnStart() const { return m_breakOnStart.value; }

        void Update();

        using ChildWidgetMap = std::map<int, std::unique_ptr<ImGuiWindow>>;

    private:
        void HandleFileExplorer();
        void HandlePerf(bool showFPS);
        void HandleBreakOnStart();

        void Serialize();
        void Deserialize();
        
        ImGuiContext* m_context;

        bool m_showFileExplorer = false;
        bool m_closeRequested = false;
        bool m_showMainMenu = true;
        Toggle m_isSoundEnabled = false;
        Toggle m_breakOnStart = false;

        Format m_currentFormat = Format::ORIGINAL;

        std::array<bool, 2> m_modes;

        std::array<bool, (size_t)Format::COUNT> m_changeFormats;
        
        inline static constexpr unsigned MAX_SAVE_STATES = 4;
        std::array<bool, MAX_SAVE_STATES> m_requestSaveState;
        std::array<bool, MAX_SAVE_STATES> m_requestLoadState;

        ChildWidgetMap m_childWidgets;
    };
}