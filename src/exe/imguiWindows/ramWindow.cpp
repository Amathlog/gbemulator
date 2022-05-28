#include "GLFW/glfw3.h"
#include "exe/messageService/messages/debugMessage.h"
#include <cstdint>
#include <cstdio>
#include <exe/messageService/messageService.h>
// #include <exe/messageService/messages/debugMessage.h>
#include <exe/imguiWindows/ramWindow.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#include <string>
#include <iostream>
#include <core/utils/utils.h>

using GBEmulatorExe::RamWindow;

RamWindow::RamWindow()
{
    m_lastUpdateTime = ImGui::GetTime();
    m_forceUpdate = true;
    m_data.resize(16 * 20);
}

void RamWindow::Draw()
{
    if (!m_open)
        return;

    double elapsedTime = ImGui::GetTime();
    constexpr double updateDeltaTime = 1.0;
    if (elapsedTime > m_lastUpdateTime + updateDeltaTime || m_forceUpdate)
    {
        m_lastUpdateTime = elapsedTime;
        m_forceUpdate = false;
        GetRamDataMessage msg(m_data.data(), m_data.size(), m_data.size(), m_addressStart);
        DispatchMessageServiceSingleton::GetInstance().Pull(msg);
    }

    static char addressStart[10] = "0000";
    if (ImGui::Begin("Ram#12", &m_open))
    {
        ImGui::Text("%s", "Go to address: 0x");
        ImGui::SameLine();
        if (ImGui::InputText("##AddressText", addressStart, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase)
            && addressStart[0] != '\0')
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
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            for (uint8_t i = 0; i < 16; ++i)
            {
                char c[2];
                GBEmulator::Utils::sprintf(c, 2, "%X", i);
                ImGui::TableSetupColumn(c);
            }
            ImGui::TableHeadersRow();

            uint16_t addr = m_addressStart;
            ImVec4 reddish(1.f, .7f, .7f, 1.f);
            ImVec4 white(1.f, 1.f, 1.f, 1.f);

            for (int row = 0; row < (int)m_data.size() / 16; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("0x%04X", addr);
                addr += 0x0010;

                for (int column = 0; column < 16; column++)
                {
                    ImGui::TableSetColumnIndex(column + 1);
                    auto value = m_data[row * 16 + column];
                    auto offset = 1.f - (float)value / 255.f;
                    ImVec4 color(1.f, offset, offset, 1.f);
                    ImGui::TextColored(color, "%02X", value);
                }
            }
            ImGui::EndTable();
        }

    }
    ImGui::End();
}
