#pragma once

#include "core/bus.h"
#include <core/z80Processor.h>
#include <cstdint>
#include <exe/messageService/message.h>
#include <string>
#include <vector>

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
    GET_CPU_REGISTERS,
    RUN_TO,
    SET_BREAK_ON_START,
    GET_OAM_ENTRIES,
    GET_GB_PALETTES,
    GET_OBJ_GBC_PALETTE,
    GET_BG_GBC_PALETTE,
    GET_VRAM
};

struct CPURegistersInfo
{
    // Registers
    GBEmulator::RegisterAF m_AF;
    GBEmulator::RegisterBC m_BC;
    GBEmulator::RegisterDE m_DE;
    GBEmulator::RegisterHL m_HL;

    uint16_t m_SP = 0x0000;
    uint16_t m_PC = 0x0000;

    // IME flag
    bool m_IMEEnabled = false;

    bool m_paused = false;
};

class DebugPayload : public GenericPayload
{
public:
    DebugPayload(DebugMessageType type, uint8_t* data, size_t dataSize, size_t dataCapacity, uint16_t addressStart,
                 uint16_t nbDisassemblyLines)
        : GenericPayload(data, dataSize, dataCapacity)
        , m_type(type)
        , m_addressStart(addressStart)
        , m_nbDisassemblyLines(nbDisassemblyLines)
    {
    }

    DebugPayload(DebugMessageType type, bool breakMode)
        : GenericPayload()
        , m_type(type)
        , m_isInBreakMode(breakMode)
    {
    }

    DebugMessageType m_type;
    uint16_t m_addressStart = 0x0000;
    std::vector<std::string> m_disassemblyLines;
    uint16_t m_nbDisassemblyLines = 0x0000;
    bool m_isInBreakMode = false;
    CPURegistersInfo m_cpuRegistersInfo;
    uint8_t m_VRAMBank = 0;
};
} // namespace GBEmulatorExe