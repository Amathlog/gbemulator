#pragma once

#include <exe/messageService/message.h>
#include <exe/messageService/messages/corePayload.h>

namespace GBEmulatorExe 
{
    using CoreMessage = TypedMessage<DefaultMessageType::CORE, CorePayload>;

    struct LoadNewGameMessage : CoreMessage
    {
        LoadNewGameMessage(std::string file)
            : CoreMessage(DefaultCoreMessageType::LOAD_NEW_GAME, file)
        {}
    };

    struct SaveGameMessage : CoreMessage
    {
        // Can be empty to get the default file
        SaveGameMessage(std::string file = "")
            : CoreMessage(DefaultCoreMessageType::SAVE_GAME, file)
        {}
    };    
    
    struct LoadSaveMessage : CoreMessage
    {
        // Can be empty to get the default file
        LoadSaveMessage(std::string file = "")
            : CoreMessage(DefaultCoreMessageType::LOAD_SAVE, file)
        {}
    };

    struct SaveStateMessage : CoreMessage
    {
        // Can be empty to get the default file
        SaveStateMessage(std::string file = "", int number = 0)
            : CoreMessage(DefaultCoreMessageType::SAVE_STATE, file, number)
        {}
    };

    struct LoadStateMessage : CoreMessage
    {
        // Can be empty to get the default file
        LoadStateMessage(std::string file = "", int number = 0)
            : CoreMessage(DefaultCoreMessageType::LOAD_STATE, file, number)
        {}
    };

    struct ChangeModeMessage : CoreMessage
    {
        ChangeModeMessage(GBEmulator::Mode mode)
            : CoreMessage(DefaultCoreMessageType::CHANGE_MODE, "", 0, mode)
        {}
    };

    struct GetModeMessage : CoreMessage
    {
        GetModeMessage()
            : CoreMessage(DefaultCoreMessageType::GET_MODE, "")
        {}
    };

    struct ResetMessage : CoreMessage
    {
        ResetMessage()
            : CoreMessage(DefaultCoreMessageType::RESET, "")
        {}
    };
}