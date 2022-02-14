#pragma once
#include "core/bus.h"
#include <string>

namespace GBEmulator 
{
    std::vector<std::string> Disassemble(GBEmulator::Bus bus, uint16_t startAddress, unsigned nbLines);
}