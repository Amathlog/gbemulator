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
#include <exe/utils.h>

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
            
            while (!m_bus.Clock()){}
            return true;
        }
        case DefaultDebugMessageType::BREAK_CONTINUE:
        {
            m_bus.BreakContinue();
            return true;
        }
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

    // Try to load an existing save
    LoadSaveGame("");

    // When all is done, restart the game
    m_bus.Reset();

    return true;
}

bool CoreMessageService::SaveState(const std::string& file, int number)
{
    std::string finalFile = file;
    if (file.empty())
    {
        finalFile = GetSaveStateFile(m_exePath, number, GetCartridgeUniqueID(m_bus.GetCartridge())).string();
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
        finalFile = GetSaveStateFile(m_exePath, number, GetCartridgeUniqueID(m_bus.GetCartridge())).string();
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
        finalFile = GetSaveFile(m_exePath, GetCartridgeUniqueID(cartridge)).string();
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
        finalFile = GetSaveFile(m_exePath, GetCartridgeUniqueID(cartridge)).string();
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