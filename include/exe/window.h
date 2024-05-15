#pragma once

#include <vector>
#include <cstdint>
#include <chrono>
#include <cmath>
#include <string>
#include <memory>
#include <type_traits>

struct GLFWwindow;

namespace GBEmulatorExe
{
    class WindowBase
    {
    public:
        virtual ~WindowBase() = default;

        virtual void Update(bool externalSync) = 0;
    
        void Enable(bool enable) {m_enable = enable;}
        bool IsEnabled() {return m_enable;}

        void SetUserData(void* userData) { m_userData = userData; }

        virtual bool RequestedClose() = 0;

    protected:
        void* m_userData = nullptr;
        bool m_isMainWindow = false;
        bool m_enable = false;
    };

    class Window : public WindowBase
    {
    public:
        Window(const char* name, unsigned width, unsigned height);
        virtual ~Window();

        void Update(bool externalSync) override;
        GLFWwindow* GetWindow() {return m_window;}


        template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Window, T>>>
        Window* CreateNewChildWindow(Args&& ...args)
        {
            m_childrenWindows.push_back(std::make_unique<T>(std::forward<Args>(args)...));
            return m_childrenWindows.back().get();
        }

        bool RequestedClose() override;
        virtual void OnScreenResized(int width, int height) {};

    protected:
        virtual void InternalUpdate(bool /*externalSync*/) {}

        GLFWwindow* m_window = nullptr;
        std::vector<std::unique_ptr<Window>> m_childrenWindows;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastUpdateTime;
    };
}