#pragma once

#include "exe/mainWindow.h"
#include <core/controller.h>
#include <cstdint>

struct GLFWwindow;

namespace GBEmulatorExe
{
    class Controller : public GBEmulator::Controller
    {
    public:
        Controller(GLFWwindow* window, uint8_t controllerIndex);
        ~Controller();

        void Update();
    private:
        GLFWwindow* m_window;
        uint8_t m_controllerIndex;
    };
}