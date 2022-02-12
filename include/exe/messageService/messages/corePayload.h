#pragma once

#include <exe/messageService/message.h>
#include <core/bus.h>
#include <string>

namespace GBEmulatorExe 
{
    using CoreMessageType = uint32_t;

    enum DefaultCoreMessageType : CoreMessageType
    {
        LOAD_NEW_GAME,
        SAVE_GAME,
        LOAD_SAVE,
        SAVE_STATE,
        LOAD_STATE,
        GET_MODE,
        CHANGE_MODE,
        RESET
    };

    class CorePayload : public Payload
    {
    public:
        CorePayload(CoreMessageType type, std::string data, int saveStateNumber = 0, GBEmulator::Mode mode = GBEmulator::Mode::GB)
            : m_type(type)
            , m_data(data)
            , m_saveStateNumber(saveStateNumber)
            , m_mode(mode)
        {}

        CoreMessageType m_type;
        std::string m_data;
        int m_saveStateNumber;
        GBEmulator::Mode m_mode;
    };
}