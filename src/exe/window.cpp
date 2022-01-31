#include <cstdint>
#include <memory>
#include <exe/window.h>
#include <exe/imguiManager.h>
#include <exe/screen.h>
#include <exe/controller.h>
#include <core/bus.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>

using GBEmulatorExe::Window;

namespace {
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        GLFWwindow* previousContext = glfwGetCurrentContext();
        glfwMakeContextCurrent(window);
        glViewport(0, 0, width, height);
        Window* myWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        myWindow->OnScreenResized(width, height);
        if (previousContext != nullptr)
            glfwMakeContextCurrent(previousContext);
    } 
}

Window::Window(const char* name, unsigned width, unsigned height)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if (m_window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    GLFWwindow* previousContext = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(m_window, this);
    framebuffer_size_callback(m_window, width, height);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    m_lastUpdateTime = std::chrono::high_resolution_clock::now();
    m_enable = true;

    if (previousContext != nullptr)
        glfwMakeContextCurrent(previousContext);
}

Window::~Window()
{
    m_childrenWindows.clear();

    glfwDestroyWindow(m_window);
    if (m_isMainWindow)
        glfwTerminate();
}

void Window::Update(bool externalSync)
{
    glfwMakeContextCurrent(m_window);
    if (RequestedClose())
        return;

    for (auto it = m_childrenWindows.begin(); it != m_childrenWindows.end();)
    {
        if ((*it)->RequestedClose())
        {
            it = m_childrenWindows.erase(it);
            continue;
        }
        else
        {
            (*it)->Update(externalSync);
        }
        ++it;
    }

    glfwMakeContextCurrent(m_window);
    glfwPollEvents();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    InternalUpdate(externalSync);

    // Swap buffers
    glfwSwapBuffers(m_window);

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>((currentTime - m_lastUpdateTime)).count();

    // if (!externalSync && diff < m_frametimeUS)
    // {
    //     std::this_thread::sleep_for(std::chrono::microseconds(m_frametimeUS - diff));
    // }

    m_lastUpdateTime = currentTime;
}

bool Window::RequestedClose()
{
    if (!m_enable || !m_window)
        return true;

    return glfwWindowShouldClose(m_window);
}