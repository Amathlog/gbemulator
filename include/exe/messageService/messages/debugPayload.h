#pragma once

#include "core/bus.h"
#include <cstdint>
#include <exe/messageService/message.h>
#include <vector>
#include <string>
#include <core/z80Processor.h>

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
        GET_CPU_REGISTERS
    };

    struct CPURegistersInfo
    {
        // Registers
        GBEmulator::RegisterAF m_AF;
        GBEmulator::RegisterBC m_BC;
        GBEmulator::RegisterDE m_DE;
        GBEmulator::RegisterHL m_HL;

        uint16_t m_SP;
        uint16_t m_PC;

        // IME flag
        bool m_IMEEnabled = false;
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
        CPURegistersInfo m_cpuRegistersInfo;
    };
}