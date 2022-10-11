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

    struct GetCPURegistersInfoMessage : DebugMessage
    {
        GetCPURegistersInfoMessage() : DebugMessage(DefaultDebugMessageType::GET_CPU_REGISTERS, nullptr, 0, 0, 0, 0)
        {}
    };

    struct RunToMessage : DebugMessage
    {
        RunToMessage(uint16_t address) : DebugMessage(DefaultDebugMessageType::RUN_TO, nullptr, 0, 0, address, 0)
        {}
    };

    struct SetBreakOnStartMessage : DebugMessage
    {
        SetBreakOnStartMessage(bool value) : DebugMessage(DefaultDebugMessageType::SET_BREAK_ON_START, value)
        {}
    };

    struct GetOAMEntriesMessage : DebugMessage
    {
        GetOAMEntriesMessage(uint8_t* data, size_t dataSize, size_t dataCapacity)
            : DebugMessage(DefaultDebugMessageType::GET_OAM_ENTRIES, data, dataSize, dataCapacity, 0, 0)
        {}
    };

    struct GetGBPaletteMessage : DebugMessage
    {
        GetGBPaletteMessage(uint8_t* data, size_t dataSize, size_t dataCapacity)
            : DebugMessage(DefaultDebugMessageType::GET_GB_PALETTES, data, dataSize, dataCapacity, 0, 0)
        {}
    };

    struct GetObjGBCPaletteMessage : DebugMessage
    {
        GetObjGBCPaletteMessage(uint8_t* data, size_t dataSize, size_t dataCapacity)
            : DebugMessage(DefaultDebugMessageType::GET_OBJ_GBC_PALETTE, data, dataSize, dataCapacity, 0, 0)
        {}
    };

    struct GetBGGBCPaletteMessage : DebugMessage
    {
        GetBGGBCPaletteMessage(uint8_t* data, size_t dataSize, size_t dataCapacity)
            : DebugMessage(DefaultDebugMessageType::GET_BG_GBC_PALETTE, data, dataSize, dataCapacity, 0, 0)
        {}
    };

    struct GetVRAMMessage : DebugMessage
    {
        GetVRAMMessage(uint8_t* data, size_t dataSize, size_t dataCapacity, uint16_t addressStart, uint8_t VRAMBank)
            : DebugMessage(DefaultDebugMessageType::GET_VRAM, data, dataSize, dataCapacity, addressStart, 0)
        {
            ownPayload.m_VRAMBank = VRAMBank;
        }
    };
}