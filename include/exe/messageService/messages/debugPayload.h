#pragma once

#include "core/bus.h"
#include <cstdint>
#include <exe/messageService/message.h>
#include <vector>
#include <string>

namespace GBEmulatorExe 
{
    using DebugMessageType = uint32_t;

    enum DefaultDebugMessageType : DebugMessageType
    {
        RAM_DATA,
        STEP,
        BREAK_CONTINUE,
        DISASSEMBLY,
        GET_BREAK_STATUS,
    };

    class DebugPayload : public GenericPayload
    {
    public:
        DebugPayload(DebugMessageType type, uint8_t* data, size_t dataSize, size_t dataCapacity, uint16_t addressStart, uint16_t nbDisassemblyLines)
            : GenericPayload(data, dataSize, dataCapacity)
            , m_type(type)
            , m_addressStart(addressStart)
            , m_nbDisassemblyLines(nbDisassemblyLines)
        {}

        DebugMessageType m_type;
        uint16_t m_addressStart = 0x0000;
        std::vector<std::string> m_disassemblyLines;
        uint16_t m_nbDisassemblyLines = 0x0000;
        bool m_isInBreakMode = false;
    };
}