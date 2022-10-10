#include "core/utils/disassenbly.h"
#include <cstdint>
#include <exe/messageService/message.h>
#include <exe/messageService/messages/coreMessage.h>
#include <exe/messageService/coreMessageService.h>
#include <exe/messageService/messages/corePayload.h>
#include <exe/messageService/messages/debugPayload.h>
#include <core/utils/fileVisitor.h>
#include <core/utils/vectorVisitor.h>
#include <core/bus.h>
#include <core/cartridge.h>
#include <filesystem>
#include <iostream>
#include <core/utils/utils.h>
#include <cstring>

namespace fs = std::filesystem;

using GBEmulatorExe::CoreMessageService;

bool CreateFolders(const std::string& file)
{
    if (file.empty())
        return true;

    auto parent = fs::path(file).parent_path();
    if (!fs::exists(parent))
    {
        if (!fs::create_directories(parent))
        {
            std::cerr << "Couldn't create the folders for file " << file << std::endl;
            return false;
        }
    }

    return true;
}

bool CoreMessageService::Push(const Message &message)
{
    if (message.GetType() != DefaultMessageType::CORE && message.GetType() != DefaultMessageType::DEBUG)
        return true;
    
    if (message.GetType() == DefaultMessageType::CORE)
    {
        auto payload = reinterpret_cast<const CorePayload*>(message.GetPayload());

        switch(payload->m_type)
        {
        case DefaultCoreMessageType::LOAD_NEW_GAME:
            return LoadNewGame(payload->m_data);
        case DefaultCoreMessageType::SAVE_GAME:
            return SaveGame(payload->m_data);
        case DefaultCoreMessageType::LOAD_SAVE:
            return LoadSaveGame(payload->m_data);
        case DefaultCoreMessageType::LOAD_STATE:
            return LoadState(payload->m_data, payload->m_saveStateNumber);
        case DefaultCoreMessageType::SAVE_STATE:
            return SaveState(payload->m_data, payload->m_saveStateNumber);
        case DefaultCoreMessageType::CHANGE_MODE:
            m_bus.ChangeMode(payload->m_mode);
            return true;
        case DefaultCoreMessageType::RESET:
            m_bus.Reset();
            return true;
        }
    }
    else if (message.GetType() == DefaultMessageType::DEBUG)
    {
        auto payload = reinterpret_cast<const DebugPayload*>(message.GetPayload());

        switch (payload->m_type)
        {
        case DefaultDebugMessageType::STEP:
        {
            if (!m_bus.IsInBreak())
                return false;
            
            bool instDone = false;
            while (!instDone){ m_bus.Clock(&instDone); }
            return true;
        }
        case DefaultDebugMessageType::BREAK_CONTINUE:
        {
            m_bus.BreakContinue();
            return true;
        }
        case DefaultDebugMessageType::RUN_TO:
        {
            m_bus.SetRunToAddress(payload->m_addressStart);
            return true;
        }
        case DefaultDebugMessageType::SET_BREAK_ON_START:
            m_bus.SetBreakOnStart(payload->m_isInBreakMode);
            return true;
        }
    }

    return true;
}

bool CoreMessageService::Pull(Message &message)
{
    if (message.GetType() != DefaultMessageType::CORE && message.GetType() != DefaultMessageType::DEBUG)
        return true;
    
    if (message.GetType() == DefaultMessageType::CORE)
    {
        auto payload = reinterpret_cast<CorePayload*>(message.GetPayload());

        switch(payload->m_type)
        {
        case DefaultCoreMessageType::GET_MODE:
            payload->m_mode = m_bus.GetMode();
            payload->m_GBModeEnabled = m_bus.IsGBModeAvailable();
            payload->m_GBCModeEnabled = m_bus.IsGBCModeAvailable();
            return true;
        }
    }
    else if (message.GetType() == DefaultMessageType::DEBUG)
    {
        auto payload = reinterpret_cast<DebugPayload*>(message.GetPayload());

        switch (payload->m_type)
        {
        case DefaultDebugMessageType::RAM_DATA:
        {
            for (uint16_t addr = 0; addr < payload->m_dataSize; ++addr)
            {
                payload->m_data[addr] = m_bus.ReadByte(payload->m_addressStart + addr);
                if (addr == 0xFFFF)
                    break;
            }
            return true;
        }
        case DefaultDebugMessageType::DISASSEMBLY:
        {
            payload->m_disassemblyLines = GBEmulator::Disassemble(m_bus, m_bus.GetCPU().GetPC(), payload->m_nbDisassemblyLines);
            return true;
        }
        case DefaultDebugMessageType::GET_BREAK_STATUS:
        {
            payload->m_isInBreakMode = m_bus.IsInBreak();
            return true;
        }
        case DefaultDebugMessageType::GET_CPU_REGISTERS:
        {
            payload->m_cpuRegistersInfo.m_AF = m_bus.GetCPU().GetAFRegister();
            payload->m_cpuRegistersInfo.m_BC = m_bus.GetCPU().GetBCRegister();
            payload->m_cpuRegistersInfo.m_DE = m_bus.GetCPU().GetDERegister();
            payload->m_cpuRegistersInfo.m_HL = m_bus.GetCPU().GetHLRegister();
            payload->m_cpuRegistersInfo.m_PC = m_bus.GetCPU().GetPC();
            payload->m_cpuRegistersInfo.m_SP = m_bus.GetCPU().GetStackPointer();
            payload->m_cpuRegistersInfo.m_IMEEnabled = m_bus.GetCPU().IsIMEEnabled();
            return true;
        }
        case DefaultDebugMessageType::GET_OAM_ENTRIES:
        {
            const auto& OAMEntries = m_bus.GetPPU().GetOAMEntries();
            const size_t OAMEntriesSize = OAMEntries.size() * sizeof(decltype(OAMEntries[0]));
            assert(payload->m_dataCapacity >= OAMEntriesSize);

            std::memcpy(payload->m_data, (uint8_t*)OAMEntries.data(), OAMEntriesSize);
            return true;
        }
        case DefaultDebugMessageType::GET_GB_PALETTES:
        {
            assert(payload->m_dataCapacity >= 3);
            payload->m_data[0] = m_bus.GetPPU().GetOAM0Palette().flags;
            payload->m_data[1] = m_bus.GetPPU().GetOAM1Palette().flags;
            payload->m_data[2] = m_bus.GetPPU().GetBGPalette().flags;

            return true;
        }
        case DefaultDebugMessageType::GET_OBJ_GBC_PALETTE:
        {
            const auto& objPalettes = m_bus.GetPPU().GetOBJPalettesGBC();
            const size_t paletteSize = objPalettes.size() * sizeof(decltype(objPalettes[0]));
            assert(payload->m_dataCapacity >= paletteSize);

            std::memcpy(payload->m_data, (uint8_t*)objPalettes.data(), paletteSize);
            return true;
        }
        case DefaultDebugMessageType::GET_BG_GBC_PALETTE:
        {
            const auto& bgPalettes = m_bus.GetPPU().GetBGPalettesGBC();
            const size_t paletteSize = bgPalettes.size() * sizeof(decltype(bgPalettes[0]));
            assert(payload->m_dataCapacity >= paletteSize);

            std::memcpy(payload->m_data, (uint8_t*)bgPalettes.data(), paletteSize);
            return true;
        }
        }
    }

    return true;
}

bool CoreMessageService::LoadNewGame(const std::string& file)
{
    // First stop the game
    // m_bus.Stop();

    // Try to save the previous game before loading
    SaveGame("");

    GBEmulator::Utils::FileReadVisitor visitor(file);

    if (!visitor.IsValid())
    {
        // m_bus.Resume();
        return false;
    }

    auto cartridge = std::make_shared<GBEmulator::Cartridge>(visitor);

    // Insert a new cartridge also reset the bus
    m_bus.InsertCartridge(cartridge);

    // Restart the game
    m_bus.Reset();

    // Try to load an existing save
    LoadSaveGame("");

    return true;
}

bool CoreMessageService::SaveState(const std::string& file, int number)
{
    std::string finalFile = file;
    if (file.empty())
    {
        finalFile = GBEmulator::Utils::GetSaveStateFile(m_exePath, number, GBEmulator::Utils::GetCartridgeUniqueID(m_bus.GetCartridge())).string();
    }

    if (finalFile.empty())
        return true;

    CreateFolders(finalFile);

    GBEmulator::Utils::FileWriteVisitor visitor(finalFile);
    if (!visitor.IsValid())
    {
        std::cerr << "Couldn't open file " << finalFile << std::endl;
        return false; 
    }

    m_bus.SerializeTo(visitor);

    return true;
}

bool CoreMessageService::LoadState(const std::string& file, int number)
{
    std::string finalFile = file;
    if (file.empty())
    {
        finalFile = GBEmulator::Utils::GetSaveStateFile(m_exePath, number, GBEmulator::Utils::GetCartridgeUniqueID(m_bus.GetCartridge())).string();
    }

    if (finalFile.empty())
        return true;

    GBEmulator::Utils::FileReadVisitor visitor(finalFile);
    if (!visitor.IsValid())
    {
        std::cerr << "Couldn't open file " << file << std::endl;
        return false; 
    }

    // When loading a state, we will reset the game then load
    // To start from a clean state.
    // To avoid multi threading issues, we stop the bus while doing so.
    // We don't want the bus to be clocked between a Reset and the deserialization
    // m_bus.Stop();

    m_bus.Reset();
    m_bus.DeserializeFrom(visitor);

    // m_bus.Resume();

    return true;
}

bool CoreMessageService::SaveGame(const std::string& file)
{
    const GBEmulator::Cartridge* cartridge = m_bus.GetCartridge();
    if (cartridge == nullptr || cartridge->GetHeader().nbRamBanks == 0)
        return true;

    std::string finalFile = file;
    if (file.empty())
    {
        finalFile = GBEmulator::Utils::GetSaveFile(m_exePath, GBEmulator::Utils::GetCartridgeUniqueID(cartridge)).string();
    }

    // Nothing to do
    if (finalFile.empty())
        return true;

    CreateFolders(finalFile);

    GBEmulator::Utils::FileWriteVisitor visitor(finalFile);
    if (!visitor.IsValid())
    {
        std::cerr << "Couldn't open file " << finalFile << std::endl;
        return false; 
    }

    m_bus.SaveCartridgeRAM(visitor);
    return true;
}

bool CoreMessageService::LoadSaveGame(const std::string& file)
{
    const GBEmulator::Cartridge* cartridge = m_bus.GetCartridge();
    if (cartridge == nullptr || cartridge->GetHeader().nbRamBanks == 0)
        return true;

    std::string finalFile = file;
    if (file.empty())
    {
        finalFile = GBEmulator::Utils::GetSaveFile(m_exePath, GBEmulator::Utils::GetCartridgeUniqueID(cartridge)).string();
    }

    if (finalFile.empty())
        return true;

    GBEmulator::Utils::FileReadVisitor visitor(finalFile);
    if (!visitor.IsValid())
    {
        std::cerr << "Couldn't open file " << file << std::endl;
        return false; 
    }

    // Loading the RAM is guarded by a lock, so there is no need to stop
    // the game.
    m_bus.LoadCartridgeRAM(visitor);
    return true;
}