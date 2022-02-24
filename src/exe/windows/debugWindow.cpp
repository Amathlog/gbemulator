#include "exe/messageService/messageService.h"
#include <exe/windows/debugWindow.h>
#include <exe/messageService/messages/debugMessage.h>
#include <exe/messageService/coreMessageService.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

using GBEmulatorExe::DebugWindow;
using GBEmulatorExe::Window;

DebugWindow::DebugWindow(unsigned width, unsigned height)
    : Window("Debug", width, height)
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

    GetBreakStatusMessage msg;
    if (DispatchMessageServiceSingleton::GetInstance().Pull(msg))
    {
        m_isInBreakMode = msg.GetTypedPayload().m_isInBreakMode;
    }
}

DebugWindow::~DebugWindow()
{
    // Cleanup
    ImGui::DestroyContext(m_context);

    GLFWwindow* previousContextGLFW = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_window);

    if (previousContextGLFW != nullptr)
        glfwMakeContextCurrent(previousContextGLFW);
}

void DebugWindow::InternalUpdate(bool externalSync)
{
    ImGui::SetCurrentContext(m_context);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    double elapsedTime = ImGui::GetTime();
    constexpr double updateDeltaTime = 0.1;
    constexpr uint16_t nbLines = 20;
    if (elapsedTime > m_lastUpdateTime + updateDeltaTime || m_forceUpdate)
    {
        m_lastUpdateTime = elapsedTime;
        m_forceUpdate = false;
        DisassemblyRequestMessage msg(nbLines);
        if (DispatchMessageServiceSingleton::GetInstance().Pull(msg))
        {
            m_data = std::move(msg.GetTypedPayload().m_disassemblyLines);
        }
    }

    static bool open = true;
    static char addressStart[10] = "0000";
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height)); 
    if (ImGui::Begin("Debug#12", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        for (auto i = 0; i < m_data.size(); ++i)
        {
            ImGui::Text("%s", m_data[i].c_str());
        }

        if (ImGui::Button(m_isInBreakMode ? "Continue" : "Break"))
        {
            DispatchMessageServiceSingleton::GetInstance().Push(BreakContinueMessage());
            m_isInBreakMode = !m_isInBreakMode;
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!m_isInBreakMode);
        if (ImGui::Button("Step"))
        {
            DispatchMessageServiceSingleton::GetInstance().Push(StepMessage());
        }
        ImGui::EndDisabled();

        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();
}

