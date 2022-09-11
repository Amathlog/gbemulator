#include <exe/imguiWindows/findRomsWindow.h>
#include <exe/messageService/messageService.h>
#include <exe/messageService/messages/coreMessage.h>
#include <core/utils/utils.h>
#include <imgui.h>
#include <iostream>

using GBEmulatorExe::FindRomsWindow;

FindRomsWindow::FindRomsWindow()
{
    m_root = GBEmulator::Utils::GetRootPath();
}

void FindRomsWindow::DrawInternal()
{
    if (m_allRoms.empty())
        RefreshRoms();

    std::filesystem::path currentParent = m_root;
    std::string currentIndent = "";

    for (auto& file : m_allRoms)
    {
        ImGui::Selectable(file.filenameWithImguiTag.c_str(), &file.selected);
    }
    
    for (auto& file : m_allRoms)
    {
        if (file.selected)
        {
            file.selected = false;
            m_open = false;
            DispatchMessageServiceSingleton::GetInstance().Push(LoadNewGameMessage(file.fullPath.string()));
        }
    }
}

void FindRomsWindow::RefreshRoms()
{
    m_allRoms.clear();
    int index = 0;

    auto findRoms = [this, &index](const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
            return;

        for (const auto& file : std::filesystem::recursive_directory_iterator(path))
        {
            if (!file.is_regular_file())
                continue;

            auto extension = file.path().extension();
            if (extension == ".gb" || extension == ".gbc")
            {
                FileEntry fileEntry;
                fileEntry.fullPath = file.path();
                fileEntry.filenameWithImguiTag = file.path().filename().string() + "##" + std::to_string(index++);
                fileEntry.selected = false;
                m_allRoms.push_back(std::move(fileEntry));
            }
        }
    };

    // Hard coded paths
    findRoms(m_root / "roms");
    findRoms(m_root / "tests");

    std::sort(m_allRoms.begin(), m_allRoms.end(), [](const FileEntry& a, const FileEntry& b)
    {
        return a.fullPath.compare(b.fullPath) < 0;
    });
}