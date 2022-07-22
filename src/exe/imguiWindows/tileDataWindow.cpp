#include "GLFW/glfw3.h"
#include "exe/messageService/messages/debugMessage.h"
#include <cstdint>
#include <cstdio>
#include <exe/messageService/messageService.h>
#include <exe/imguiWindows/tileDataWindow.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#include <string>
#include <iostream>
#include <core/utils/utils.h>
#include <core/utils/tile.h>
#include <cstring>

using GBEmulatorExe::TileDataWindow;

TileDataWindow::TileDataWindow()
{
    m_lastUpdateTime = ImGui::GetTime();
    m_forceUpdate = true;
    m_data.resize(128 * 16 * 2);

    m_image = std::make_unique<Image>(16 * 8, 16 * 8); // 16 * 16 tiles
}

void TileDataWindow::UpdateImage()
{
    std::vector<uint8_t>& finalImage = m_image->GetInternalBuffer();

    std::array<uint8_t, 12> paletteColors;
    int i = 0;
    for (auto& color : { GBEmulator::WHITE_COLOR, GBEmulator::LIGHT_GREY_COLOR, GBEmulator::DARK_GREY_COLOR, GBEmulator::BLACK_COLOR })
    {
        GBEmulator::Utils::RGB555ToRGB888(color, paletteColors[i], paletteColors[i+1], paletteColors[i+2]);
        i += 3;
    }

    for (auto i = 0; i < 256; ++i)
    {
        std::array<uint8_t, 64> paletteColorIndexes = GBEmulator::Utils::GetTileDataFromBytes(m_data.data() + i * 16);
        int tileLine = i / 16;
        int tileColumn = i % 16;

        for (auto j = 0; j < 64; ++j)
        {
            int line = j / 8;
            int column = 7 - j % 8;

            int finalLine = tileLine * 8 + line;
            int finalColumn = tileColumn * 8 + column;

            std::memcpy(finalImage.data() + (finalLine * 128 + finalColumn) * 3, paletteColors.data() + 3 * paletteColorIndexes[j], 3);
        }
    }

    m_image->UpdateGLTexture(true);
}

void TileDataWindow::Draw()
{
    if (!m_open)
        return;

    double elapsedTime = ImGui::GetTime();
    constexpr double updateDeltaTime = 1.0;
    if (elapsedTime > m_lastUpdateTime + updateDeltaTime || m_forceUpdate)
    {
        m_lastUpdateTime = elapsedTime;
        m_forceUpdate = false;
        GetRamDataMessage msg(m_data.data(), m_data.size(), m_data.size(), 0x8000 + m_currentBlock * 0x0800);
        DispatchMessageServiceSingleton::GetInstance().Pull(msg);

        UpdateImage();
    }

    static std::array<bool, 3> m_checkBoxesBlocks = {true, false};

    if (ImGui::Begin("TileData#12", &m_open))
    {
        ImGui::Text("%s", "Select a block: ");
        ImGui::SameLine();
        ImGui::Checkbox("Start address 0x8000", &m_checkBoxesBlocks[0]);
        ImGui::SameLine();
        ImGui::Checkbox("Start address 0x8800", &m_checkBoxesBlocks[1]);

        for (auto i = 0; i < 3; i++)
        {
            if (m_checkBoxesBlocks[i] && i != m_currentBlock)
            {
                m_checkBoxesBlocks[m_currentBlock] = false;
                m_currentBlock = i;
                break;
            }
        }

        ImGui::Image((void*)(intptr_t)m_image->GetTextureId(), ImVec2(16*8*5, 16*8*5));

    }
    ImGui::End();
}
