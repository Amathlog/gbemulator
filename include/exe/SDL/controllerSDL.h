#pragma once

#include <core/controller.h>

union SDL_Event;

namespace GBEmulatorExe
{
    class ControllerSDL : public GBEmulator::Controller
    {
    public:
        ControllerSDL() = default;
        virtual ~ControllerSDL() = default;

        void Update(const SDL_Event& event);
    };
}