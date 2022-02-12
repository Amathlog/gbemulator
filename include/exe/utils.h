#pragma once

#include <core/cartridge.h>
#include <filesystem>

namespace GBEmulatorExe 
{
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
}