#include "GLFW/glfw3.h"
#include "exe/messageService/messages/debugMessage.h"
#include <cstdint>
#include <cstdio>
#include <exe/messageService/messageService.h>
// #include <exe/messageService/messages/debugMessage.h>
#include <exe/windows/ramWindow.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#include <string>
#include <iostream>

using GBEmulatorExe::RamWindow;
using GBEmulatorExe::Window;

RamWindow::RamWindow(unsigned width, unsigned height)
    : Window("RAM", width, height)
    , m_width(width)
    , m_height(height)
{
    GLFWwindow* previousContextGLFW = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext* previousContext = ImGui::GetCurrentContext();
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_context);
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    if (previousContext != nullptr)
        ImGui::SetCurrentContext(previousContext);

    if (previousContextGLFW != nullptr)
        glfwMakeContextCurrent(previousContextGLFW);

    m_lastUpdateTime = ImGui::GetTime();
    m_forceUpdate = true;
    m_data.resize(16 * 20);
}

RamWindow::~RamWindow()
{
    // Cleanup
    ImGui::DestroyContext(m_context);

    GLFWwindow* previousContextGLFW = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_window);

    if (previousContextGLFW != nullptr)
        glfwMakeContextCurrent(previousContextGLFW);
}

void RamWindow::InternalUpdate(bool externalSync)
{
    ImGui::SetCurrentContext(m_context);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    double elapsedTime = ImGui::GetTime();
    constexpr double updateDeltaTime = 1.0;
    if (elapsedTime > m_lastUpdateTime + updateDeltaTime || m_forceUpdate)
    {
        m_lastUpdateTime = elapsedTime;
        m_forceUpdate = false;
        GetRamDataMessage msg(m_data.data(), m_data.size(), m_data.size(), m_addressStart);
        DispatchMessageServiceSingleton::GetInstance().Pull(msg);
    }

    static bool open = true;
    static char addressStart[10] = "0000";
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height)); 
    if (ImGui::Begin("Ram#12", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        ImGui::Text("%s", "Go to address: 0x");
        ImGui::SameLine();
        ImGui::InputText("##AddressText", addressStart, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::SameLine();
        if (ImGui::Button("Apply") && addressStart[0] != '\0')
        {
            m_forceUpdate = true;
            auto addressStartN = std::stoul(addressStart, 0, 16);
            m_addressStart = (addressStartN & 0xFFF0);
            if (m_addressStart > 0xFFF0 - 0x0130)
            {
                m_addressStart = 0xFFF0 - 0x0130;
            }
        }
        
        if (ImGui::BeginTable("Table", 17))
        {
            ImGui::TableSetupColumn("Addr");
            for (uint8_t i = 0; i < 16; ++i)
            {
                char c[2];
                std::sprintf(c, "%X", i);
                ImGui::TableSetupColumn(c);
            }
            ImGui::TableHeadersRow();

            uint16_t addr = m_addressStart;

            for (int row = 0; row < (int)m_data.size() / 16; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("0x%04X", addr);
                addr += 0x0010;

                for (int column = 0; column < 16; column++)
                {
                    ImGui::TableSetColumnIndex(column + 1);
                    ImGui::Text("%02X", m_data[row * 16 + column]);
                }
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();
}
