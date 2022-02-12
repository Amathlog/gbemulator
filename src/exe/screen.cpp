#include <exe/screen.h>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <exe/messageService/screenMessageService.h>

using GBEmulatorExe::Screen;

Screen::Screen(unsigned internalResWidth, unsigned internalResHeight)
    : m_image(internalResWidth, internalResHeight)
{
    if (!m_image.IsInitialized())
        return;

    m_initialized = true;

    m_screenMessageService = std::make_unique<ScreenMessageService>(*this);
    DispatchMessageServiceSingleton::GetInstance().Connect(m_screenMessageService.get());

    m_lastTick = std::chrono::high_resolution_clock::now();
}

Screen::~Screen()
{
    DispatchMessageServiceSingleton::GetInstance().Disconnect(m_screenMessageService.get());
}

void Screen::Update()
{
    if (!m_initialized)
        return;

    if (m_image.BufferWasUpdated())
    {
        auto newTick = std::chrono::high_resolution_clock::now();
        float frametime = (float)std::chrono::duration_cast<std::chrono::milliseconds>(newTick - m_lastTick).count();
        m_lastTick = newTick;
        m_frametimes[m_frametimeOffset] = frametime;
        m_frametimeOffset = (m_frametimeOffset + 1) % m_frametimes.size();
    }

    m_image.Draw();
}

void Screen::OnScreenResized(int width, int height)
{
    m_image.UpdateRatio(width, height);
}