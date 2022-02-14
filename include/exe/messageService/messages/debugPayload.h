#pragma once

#include <exe/messageService/message.h>

namespace GBEmulatorExe 
{
    using DebugMessageType = uint32_t;

    enum DefaultDebugMessageType : DebugMessageType
    {
        RAM_DATA,
        STEP,
        BREAK_CONTINUE,
        DISASSEMBLY
    };

    class DebugPayload : public GenericPayload
    {
    public:
        DebugPayload(DebugMessageType type, uint8_t* data, size_t dataSize, size_t dataCapacity, uint16_t addressStart)
            : GenericPayload(data, dataSize, dataCapacity)
            , m_type(type)
            , m_addressStart(addressStart)
        {}

        DebugMessageType m_type;
        uint16_t m_addressStart;
    };
}