#pragma once

#include <core/cartridge.h>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace GBEmulatorExe 
{
    static constexpr const char* ROOT_FILE = "gbemulator_root.myroot";

    inline std::string GetCartridgeUniqueID(const GBEmulator::Cartridge* cartridge)
    {
        if (cartridge != nullptr)
            return cartridge->GetSHA1();

        return "";
    }

    inline std::filesystem::path GetSaveFolder(std::filesystem::path exeDir, std::string uniqueID)
    {
        return exeDir / "saves" / uniqueID;
    }

    inline std::filesystem::path GetSaveStateFile(std::filesystem::path exeDir, int number, std::string uniqueID)
    {
        if (uniqueID.empty())
            return std::filesystem::path();

        return GetSaveFolder(exeDir, uniqueID) / (std::to_string(number) + ".nesSaveState");
    }

    inline std::filesystem::path GetSaveFile(std::filesystem::path exeDir, std::string uniqueID)
    {
        if (uniqueID.empty())
            return std::filesystem::path();

        return GetSaveFolder(exeDir, uniqueID) / "save.nesSave";
    }

#ifdef _WIN32
    inline std::filesystem::path GetExePath()
    {
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
    }
#else
    inline std::filesystem::path GetExePath()
    {
        return std::filesystem::canonical("/proc/self/exe").parent_path();
    }
#endif

    inline std::filesystem::path GetRootPath()
    {
        std::filesystem::path exePath = GetExePath();

        std::filesystem::path curr = exePath;
        bool found = false;

        while (!found)
        {
            if (!curr.has_parent_path())
                break;
            
            for (const auto& file : std::filesystem::directory_iterator(curr))
            {
                if (!file.is_regular_file())
                    continue;

                if (file.path().filename() == ROOT_FILE)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                curr = curr.parent_path();
        }

        return found ? curr : exePath;
    }
}