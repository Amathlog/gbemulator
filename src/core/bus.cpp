#include "core/z80Processor.h"
#include <core/bus.h>

using GBEmulator::Bus;

void Bus::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    m_cpu.SerializeTo(visitor);
}

void Bus::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    m_cpu.DeserializeFrom(visitor);
}

void Bus::Reset()
{
    m_cpu.Reset();
}