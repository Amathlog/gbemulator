#pragma once

#include <exe/window.h>
#include <exe/screen.h>
#include <exe/imguiManager.h>

struct SDL_Window;

namespace GBEmulatorExe
{
    class ControllerSDL;

    class MainWindowSDL : public WindowBase
    {
    public:
        MainWindowSDL(const char* name, unsigned width, unsigned height, unsigned internalResWidth, unsigned internalResHeight);
        ~MainWindowSDL();

        SDL_Window* GetWindow() {return m_window;}
        void* GetGLContext() { return m_glContext; }

        bool RequestedClose() override;

        void Update(bool externalSync) override;

        // Need to be done after setting a bus
        void ConnectController();

        const ImguiManager* GetImguiManager() const { return m_imguiManager.get(); }

    private:
        std::shared_ptr<ControllerSDL> m_controller = nullptr;

        std::unique_ptr<ImguiManager> m_imguiManager;
        std::unique_ptr<Screen> m_screen;

        SDL_Window* m_window = nullptr;
        void* m_glContext = nullptr;

        bool m_requestedClose = false;
    };
}