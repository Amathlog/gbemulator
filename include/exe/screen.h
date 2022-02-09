#pragma once

// #include <new_exe/messageService/messages/screenPayload.h>
#include <exe/rendering/image.h>
#include <memory>
#include <vector>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <array>

namespace GBEmulatorExe
{
    // class ScreenMessageService;
    
    class Screen
    {
    public:

        Screen(unsigned internalResWidth, unsigned internalResHeight);
        ~Screen();

        void Update();
        bool IsInitialized() const { return m_initialized; }
        void OnScreenResized(int width, int height);
        Image& GetImage() { return m_image; }

        const float* GetFrametimes(size_t& offset, size_t& size) const
        {
            offset = m_frametimeOffset;
            size = m_frametimes.size();
            return m_frametimes.data();
        }

    private:

        Image m_image;
        bool m_initialized = false;

        // std::unique_ptr<ScreenMessageService> m_screenMessageService;

        std::array<float, 100> m_frametimes = {};
        size_t m_frametimeOffset = 0;
        std::chrono::high_resolution_clock::time_point m_lastTick;
    };
}