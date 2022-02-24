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

        GetCPURegistersInfoMessage msg2;
        if (DispatchMessageServiceSingleton::GetInstance().Pull(msg2))
        {
            m_cpuRegisterInfo = std::move(msg2.GetTypedPayload().m_cpuRegistersInfo);
        }
    }

    static bool open = true;
    static char addressStart[10] = "0000";
    int limit = 2 * m_width / 5;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(limit, m_height));
    if (ImGui::Begin("Disassembly#12", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
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

    ImGui::SetNextWindowPos(ImVec2(limit, 0));
    ImGui::SetNextWindowSize(ImVec2(m_width - limit, m_height));

    if(ImGui::Begin("Status#12", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        const ImVec4 greenColor(0.f, 1.f, 0.f, 1.f); 
        const ImVec4 redColor(1.f, 0.f, 0.f, 1.f);

        ImGui::Text("Flags: ");
        ImGui::SameLine();
        ImGui::TextColored(m_cpuRegisterInfo.m_AF.F.Z ? greenColor : redColor, "%c  ", 'Z');
        ImGui::SameLine();
        ImGui::TextColored(m_cpuRegisterInfo.m_AF.F.H ? greenColor : redColor, "%c  ", 'H');
        ImGui::SameLine();
        ImGui::TextColored(m_cpuRegisterInfo.m_AF.F.N ? greenColor : redColor, "%c  ", 'N');
        ImGui::SameLine();
        ImGui::TextColored(m_cpuRegisterInfo.m_AF.F.C ? greenColor : redColor, "%c  ", 'C');
        ImGui::SameLine();
        ImGui::TextColored(m_cpuRegisterInfo.m_IMEEnabled ? greenColor : redColor, "%s", "IME");

        ImGui::Text("A: 0x%02X [%3d]", m_cpuRegisterInfo.m_AF.A, m_cpuRegisterInfo.m_AF.A);
        ImGui::Text("BC: 0x%04X ; B: 0x%02X [%3d] ; C: 0x%02X [%3d]", m_cpuRegisterInfo.m_BC.BC, 
                                                                        m_cpuRegisterInfo.m_BC.B, m_cpuRegisterInfo.m_BC.B, 
                                                                        m_cpuRegisterInfo.m_BC.C, m_cpuRegisterInfo.m_BC.C);
        ImGui::Text("DE: 0x%04X ; D: 0x%02X [%3d] ; E: 0x%02X [%3d]", m_cpuRegisterInfo.m_DE.DE, 
                                                                        m_cpuRegisterInfo.m_DE.D, m_cpuRegisterInfo.m_DE.D, 
                                                                        m_cpuRegisterInfo.m_DE.E, m_cpuRegisterInfo.m_DE.E);
        ImGui::Text("HL: 0x%04X ; H: 0x%02X [%3d] ; L: 0x%02X [%3d]", m_cpuRegisterInfo.m_HL.HL, 
                                                                        m_cpuRegisterInfo.m_HL.H, m_cpuRegisterInfo.m_HL.H, 
                                                                        m_cpuRegisterInfo.m_HL.L, m_cpuRegisterInfo.m_HL.L);
        ImGui::Text("SP: 0x%04X", m_cpuRegisterInfo.m_SP);
        ImGui::Text("PC: 0x%04X", m_cpuRegisterInfo.m_PC);

        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();
}

