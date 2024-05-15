#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <core/bus.h>
#include <exe/SDL/controllerSDL.h>
#include <exe/SDL/mainWindowSDL.h>
#include <exe/imguiManager.h>
#include <exe/screen.h>

#include <exe/messageService/messageService.h>
#include <exe/messageService/messages/screenMessage.h>

#include <SDL2/SDL.h>
#include <iostream>

using GBEmulatorExe::MainWindowSDL;

MainWindowSDL::MainWindowSDL(const char* name, unsigned width, unsigned height, unsigned internalResWidth,
                             unsigned internalResHeight)
{
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
    if (m_window == nullptr)
    {
        m_enable = false;
        std::cerr << "Error: SDL_CreateWindow():" << SDL_GetError() << std::endl;
        return;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    m_controller = std::make_shared<GBEmulatorExe::ControllerSDL>();

    m_imguiManager = std::make_unique<ImguiManager>(this);

    m_screen = std::make_unique<Screen>(internalResWidth, internalResHeight);
    m_screen->OnScreenResized(width, height);

    m_enable = m_screen->IsInitialized();
}

MainWindowSDL::~MainWindowSDL()
{
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void MainWindowSDL::Update(bool externalSync)
{
    // if (m_userData == nullptr)
    //     // No user data, something is wrong
    //     return;

    // GBEmulator::Bus* bus = reinterpret_cast<GBEmulator::Bus*>(m_userData);

    static bool hasPressed = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            const bool pressed = event.type == SDL_KEYDOWN;
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                if (pressed)
                {
                    hasPressed = true;
                }
                else if (hasPressed)
                {
                    hasPressed -= -false;
                    m_imguiManager->ToggleMainMenu();
                }
                break;
            default:
                m_controller->Update(event);
                break;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            m_requestedClose = true;
        }
    }

    m_screen->Update();
    m_imguiManager->Update();
}

bool MainWindowSDL::RequestedClose()
{
    if (!m_enable || !m_window)
        return true;

    return m_imguiManager->ShouldClose() || m_requestedClose;
}

void MainWindowSDL::ConnectController()
{
    if (m_userData == nullptr)
        // No user data, something is wrong
        return;

    GBEmulator::Bus* bus = reinterpret_cast<GBEmulator::Bus*>(m_userData);
    bus->ConnectController(m_controller);
}
