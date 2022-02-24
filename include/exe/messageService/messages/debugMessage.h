#pragma once

#include <cstdint>
#include <exe/messageService/messages/debugPayload.h>
#include <exe/messageService/message.h>

namespace GBEmulatorExe 
{
    using DebugMessage = TypedMessage<DefaultMessageType::DEBUG, DebugPayload>;

    struct GetRamDataMessage : DebugMessage
    {
        GetRamDataMessage(uint8_t* data, size_t dataSize, size_t dataCapacity, uint16_t addressStart)
            : DebugMessage(DefaultDebugMessageType::RAM_DATA, data, dataSize, dataCapacity, addressStart, 0)
        {}
    };

    struct StepMessage : DebugMessage
    {
        StepMessage() : DebugMessage(DefaultDebugMessageType::STEP, nullptr, 0, 0, 0, 0)
        {}
    };

    struct BreakContinueMessage : DebugMessage
    {
        BreakContinueMessage() : DebugMessage(DefaultDebugMessageType::BREAK_CONTINUE, nullptr, 0, 0, 0, 0)
        {}
    };

    struct DisassemblyRequestMessage : DebugMessage
    {
        DisassemblyRequestMessage(uint16_t nbDisassemblyLines)
            : DebugMessage(DefaultDebugMessageType::DISASSEMBLY, nullptr, 0, 0, 0, nbDisassemblyLines)
        {}
    };

    struct GetBreakStatusMessage : DebugMessage
    {
        GetBreakStatusMessage() : DebugMessage(DefaultDebugMessageType::GET_BREAK_STATUS, nullptr, 0, 0, 0, 0)
        {}
    };
}