#pragma once

#include <exe/imguiWindows/imguiWindow.h>
#include <exe/window.h>
#include <filesystem>
#include <memory>
#include <string>

struct ImGuiContext;

struct FileEntry
{
    std::filesystem::path fullPath;
    std::string filenameWithImguiTag;
    bool selected;
};

namespace GBEmulatorExe
{
class FindRomsWindow : public ImGuiWindow
{
public:
    FindRomsWindow();

    WINDOW_ID_IMPL(AllWindowsId::FindRomsWindowId);

protected:
    void RefreshRoms();
    void DrawInternal() override;
    const char* GetWindowName() const override { return "Find Roms##12"; }

    std::vector<FileEntry> m_allRoms;
    std::filesystem::path m_root;
};
} // namespace GBEmulatorExe