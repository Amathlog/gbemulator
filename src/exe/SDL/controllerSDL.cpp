#include <SDL2/SDL_keycode.h>
#include <exe/SDL/controllerSDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

using GBEmulatorExe::ControllerSDL;

void ControllerSDL::Update(const SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        const bool pressed = event.type == SDL_KEYDOWN;
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            ToggleUp(pressed);
            break;
        case SDLK_DOWN:
            ToggleDown(pressed);
            break;
        case SDLK_LEFT:
            ToggleLeft(pressed);
            break;
        case SDLK_RIGHT:
            ToggleRight(pressed);
            break;
        case SDLK_z:
            ToggleA(pressed);
            break;
        case SDLK_x:
            ToggleB(pressed);
            break;
        case SDLK_a:
            ToggleStart(pressed);
            break;
        case SDLK_s:
            ToggleSelect(pressed);
            break;
        default:
            break;
        }
    }
}