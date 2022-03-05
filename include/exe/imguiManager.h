#pragma once

#include "exe/imguiWindows/imguiWindow.h"
#include <string>
#include <exe/rendering/image.h>
#include <array>
#include <map>


struct ImGuiContext;

namespace GBEmulatorExe
{
    class Window;
    
    class ImguiManager
    {
    public:
        ImguiManager(Window* window);
        ~ImguiManager();

        bool ShouldClose() const { return m_closeRequested; }
        void ToggleMainMenu() { m_showMainMenu = !m_showMainMenu; }

        void Update();

    private:
        void HandleFileExplorer();
        void HandlePerf(bool showFPS);
        
        Window* m_window;
        ImGuiContext* m_context;

        bool m_showFileExplorer = false;
        bool m_closeRequested = false;
        bool m_showMainMenu = true;
        bool m_isSoundEnabled = false;
        bool m_previousSoundState = false;

        Format m_currentFormat = Format::ORIGINAL;

        std::array<bool, (size_t)Format::COUNT> m_changeFormats;
        
        inline static constexpr unsigned MAX_SAVE_STATES = 4;
        std::array<bool, MAX_SAVE_STATES> m_requestSaveState;
        std::array<bool, MAX_SAVE_STATES> m_requestLoadState;

        std::map<int, std::unique_ptr<ImGuiWindow>> m_childWidgets;
    };
}