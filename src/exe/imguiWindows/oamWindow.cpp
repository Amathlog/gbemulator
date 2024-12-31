#include "GLFW/glfw3.h"
#include "exe/messageService/messages/coreMessage.h"
#include "exe/messageService/messages/debugMessage.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <core/constants.h>
#include <core/utils/tile.h>
#include <core/utils/utils.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exe/imguiWindows/oamWindow.h>
#include <exe/messageService/messageService.h>
#include <imgui.h>
#include <iostream>
#include <sstream>
#include <string>

using GBEmulatorExe::OAMWindow;

namespace
{
ImColor GetImGuiColor(GBEmulator::RGB555 color)
{
    uint8_t r, g, b;
    GBEmulator::Utils::RGB555ToRGB888(color, r, g, b);

    return ImColor((int)r, (int)g, (int)b);
}

constexpr float colorSize = 18.0f;
constexpr float outlineThickness = 1.0f;
constexpr float outlineSizeX = colorSize * 4 + 2 * outlineThickness;
constexpr float outlineSizeY = colorSize + 2 * outlineThickness;
constexpr float spacing = 10.0f;
} // namespace

void OAMWindow::DrawGBPalette()
{
    const auto& gbPalette = GBEmulator::GB_ORIGINAL_PALETTE;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::Text("%s", "Palette");

    const ImVec2 p = ImGui::GetCursorScreenPos();
    float x = p.x + 4.0f;
    float y = p.y + 4.0f;

    for (auto i = 0; i < 2; ++i)
    {
        // Outline
        draw_list->AddRect(ImVec2(x, y), ImVec2(x + outlineSizeX, y + outlineSizeY), ImColor(0xFFFFFFFF), 0.0f, 0,
                           outlineThickness);
        x += outlineThickness;
        y += outlineThickness;

        GBEmulator::GBPaletteData palette = (i == 0 ? m_gbOBJ0Palette : m_gbOBJ1Palette);

        ImColor color = GetImGuiColor(gbPalette[palette.color0]);
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + colorSize, y + colorSize), color);
        x += colorSize;

        color = GetImGuiColor(gbPalette[palette.color1]);
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + colorSize, y + colorSize), color);
        x += colorSize;

        color = GetImGuiColor(gbPalette[palette.color2]);
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + colorSize, y + colorSize), color);
        x += colorSize;

        color = GetImGuiColor(gbPalette[palette.color3]);
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + colorSize, y + colorSize), color);
        x += colorSize;

        x += spacing;
        y -= outlineThickness;
    }

    y += outlineSizeY + spacing;

    ImGui::SetCursorScreenPos(ImVec2(p.x, y));
}

void OAMWindow::DrawGBCPalette()
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 p = ImGui::GetCursorScreenPos();
    float x = p.x + 4.0f;
    float y = p.y + 4.0f;

    auto DrawColor = [this, &x, &y, draw_list](const GBEmulator::GBCPaletteData& palette)
    {
        // Outline
        draw_list->AddRect(ImVec2(x, y), ImVec2(x + outlineSizeX, y + outlineSizeY), ImColor(0xFFFFFFFF), 0.0f, 0,
                           outlineThickness);
        x += outlineThickness;
        y += outlineThickness;

        for (auto j = 0; j < 4; ++j)
        {
            ImColor color = GetImGuiColor(palette.colors[j]);

            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + colorSize, y + colorSize), color);
            x += colorSize;
        }

        x += spacing;
        y -= outlineThickness;
    };

    draw_list->AddText(ImVec2(x, y), ImColor(0xFFFFFFFF), "OBJ palette:");

    y += 25.f;

    for (auto i = 0; i < m_gbcOBJPalettes.size(); ++i)
    {
        DrawColor(m_gbcOBJPalettes[i]);
    }

    x = p.x + 4.0f;
    y += outlineSizeY + spacing;

    draw_list->AddText(ImVec2(x, y), ImColor(0xFFFFFFFF), "BG palette:");

    y += 25.f;

    for (auto i = 0; i < m_gbcBGPalettes.size(); ++i)
    {
        DrawColor(m_gbcBGPalettes[i]);
    }

    y += outlineSizeY + spacing;

    ImGui::SetCursorScreenPos(ImVec2(p.x, y));
}

OAMWindow::OAMWindow()
{
    m_lastUpdateTime = ImGui::GetTime();
    m_forceUpdate = true;
    // m_data.resize(128 * 16 * 2);

    // m_image = std::make_unique<Image>(16 * 8, 16 * 8); // 16 * 16 tiles
}

void OAMWindow::UpdateImage()
{
    // std::vector<uint8_t>& finalImage = m_image->GetInternalBuffer();

    // std::array<uint8_t, 12> paletteColors;
    // int i = 0;
    // for (auto& color : { GBEmulator::WHITE_COLOR, GBEmulator::LIGHT_GREY_COLOR, GBEmulator::DARK_GREY_COLOR,
    // GBEmulator::BLACK_COLOR })
    // {
    //     GBEmulator::Utils::RGB555ToRGB888(color, paletteColors[i], paletteColors[i+1], paletteColors[i+2]);
    //     i += 3;
    // }

    // for (auto i = 0; i < 256; ++i)
    // {
    //     std::array<uint8_t, 64> paletteColorIndexes = GBEmulator::Utils::GetTileDataFromBytes(m_data.data() + i *
    //     16); int tileLine = i / 16; int tileColumn = i % 16;

    //     for (auto j = 0; j < 64; ++j)
    //     {
    //         int line = j / 8;
    //         int column = 7 - j % 8;

    //         int finalLine = tileLine * 8 + line;
    //         int finalColumn = tileColumn * 8 + column;

    //         std::memcpy(finalImage.data() + (finalLine * 128 + finalColumn) * 3, paletteColors.data() + 3 *
    //         paletteColorIndexes[j], 3);
    //     }
    // }

    // m_image->UpdateGLTexture(true);
}

void OAMWindow::DrawInternal()
{
    double elapsedTime = ImGui::GetTime();
    constexpr double updateDeltaTime = 0.1;
    if (elapsedTime > m_lastUpdateTime + updateDeltaTime || m_forceUpdate)
    {
        m_lastUpdateTime = elapsedTime;
        m_forceUpdate = false;
        const size_t OAMEntriesSize = m_oamEntries.size() * sizeof(decltype(m_oamEntries[0]));
        GetOAMEntriesMessage msg((uint8_t*)m_oamEntries.data(), 0, OAMEntriesSize);
        DispatchMessageServiceSingleton::GetInstance().Pull(msg);

        GetModeMessage modeMsg;
        DispatchMessageServiceSingleton::GetInstance().Pull(modeMsg);
        m_isGBC = modeMsg.GetTypedPayload().m_mode == GBEmulator::Mode::GBC;

        if (!m_isGBC)
        {
            GBEmulator::GBPaletteData objPaletteData[3];
            GetGBPaletteMessage paletteMsg((uint8_t*)objPaletteData, 0, 3);
            DispatchMessageServiceSingleton::GetInstance().Pull(paletteMsg);

            m_gbOBJ0Palette.flags = objPaletteData[0].flags;
            m_gbOBJ1Palette.flags = objPaletteData[1].flags;
        }
        else
        {
            const size_t paletteSize = m_gbcOBJPalettes.size() * sizeof(decltype(m_gbcOBJPalettes[0]));
            GetObjGBCPaletteMessage paletteMsg((uint8_t*)m_gbcOBJPalettes.data(), 0, paletteSize);
            DispatchMessageServiceSingleton::GetInstance().Pull(paletteMsg);

            GetBGGBCPaletteMessage paletteBGMsg((uint8_t*)m_gbcBGPalettes.data(), 0, paletteSize);
            DispatchMessageServiceSingleton::GetInstance().Pull(paletteBGMsg);
        }

        UpdateImage();
    }

    constexpr const char* ColumnsNames[] = {"Id", "Sprite", "Position", "Palette", "XFlip", "YFlip", "BGPriority"};

    if (m_isGBC)
    {
        DrawGBCPalette();
    }
    else
    {
        DrawGBPalette();
    }

    ImGui::BeginTable("##OAMEntries", 7);
    ImGui::TableNextRow();
    for (auto j = 0; j < 7; ++j)
    {
        ImGui::TableNextColumn();
        ImGui::Text("%s", ColumnsNames[j]);
    }

    for (auto i = 0; i < m_oamEntries.size(); ++i)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%d", i);

        ImGui::TableNextColumn();
        ImGui::Text("%d", m_oamEntries[i].tileIndex);

        ImGui::TableNextColumn();
        ImGui::Text("(%d, %d)", m_oamEntries[i].xPosition, m_oamEntries[i].yPosition);

        ImGui::TableNextColumn();
        ImGui::Text(
            "%d", (m_isGBC ? m_oamEntries[i].attributes.paletteNumberGBC : m_oamEntries[i].attributes.paletteNumberGB));

        ImGui::TableNextColumn();
        ImGui::Text("%s", (m_oamEntries[i].attributes.xFlip ? "X" : " "));

        ImGui::TableNextColumn();
        ImGui::Text("%s", (m_oamEntries[i].attributes.yFlip ? "X" : " "));

        ImGui::TableNextColumn();
        ImGui::Text("%s", (m_oamEntries[i].attributes.bgAndWindowOverObj ? "X" : " "));
    }
    ImGui::EndTable();

    // static std::array<bool, 3> m_checkBoxesBlocks = {true, false};

    // ImGui::Text("%s", "Select a block: ");
    // ImGui::SameLine();
    // ImGui::Checkbox("Start address 0x8000", &m_checkBoxesBlocks[0]);
    // ImGui::SameLine();
    // ImGui::Checkbox("Start address 0x8800", &m_checkBoxesBlocks[1]);

    // for (auto i = 0; i < 3; i++)
    // {
    //     if (m_checkBoxesBlocks[i] && i != m_currentBlock)
    //     {
    //         m_checkBoxesBlocks[m_currentBlock] = false;
    //         m_currentBlock = i;
    //         break;
    //     }
    // }

    // ImGui::Image((void*)(intptr_t)m_image->GetTextureId(), ImVec2(16*8*5, 16*8*5));
}
