#include "exe/imguiWindows/imguiWindow.h"
#include "exe/messageService/messageService.h"
#include <exe/imguiWindows/debugWindow.h>
#include <exe/messageService/messages/debugMessage.h>
#include <exe/messageService/coreMessageService.h>
#include <imgui.h>

using GBEmulatorExe::DebugWindow;

DebugWindow::DebugWindow()
{
    m_lastUpdateTime = ImGui::GetTime();
    m_forceUpdate = true;

    UpdateBreakStatus();
}

void DebugWindow::UpdateBreakStatus()
{
    GetBreakStatusMessage msg;
    if (DispatchMessageServiceSingleton::GetInstance().Pull(msg))
    {
        m_isInBreakMode = msg.GetTypedPayload().m_isInBreakMode;
    }
}

void DebugWindow::Draw()
{
    if (!m_open)
        return;

    if (ImGui::Begin("DebugWindow", &m_open))
    {
        m_width = ImGui::GetWindowWidth();
        m_height = ImGui::GetWindowHeight();

        ImVec2 windowPos = ImGui::GetWindowPos();
        windowPos.x += ImGui::GetWindowContentRegionMin().x;
        windowPos.y += ImGui::GetWindowContentRegionMin().y;

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
        float limit = 2.f * m_width / 5.f;
        ImGui::SetNextWindowPos(windowPos);
        if (ImGui::BeginChild("Disassembly#12", ImVec2(limit, m_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
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

        }

        ImGui::EndChild();

        ImVec2 nextWindowPos = ImVec2(windowPos.x + limit, windowPos.y);
        ImVec2 separatorLineStart = ImVec2(nextWindowPos.x - 10, nextWindowPos.y);
        ImVec2 separatorLineEnd = ImVec2(separatorLineStart.x, nextWindowPos.y + ImGui::GetWindowContentRegionMax().y);
        ImGui::GetWindowDrawList()->AddLine(separatorLineStart, separatorLineEnd, 0xFFFFFFFF, 0.5f);

        ImGui::SetNextWindowPos(nextWindowPos);

        if(ImGui::BeginChild("Status#12", ImVec2(m_width - limit, m_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
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

        }

        ImGui::EndChild();
    }

    ImGui::End();
}

