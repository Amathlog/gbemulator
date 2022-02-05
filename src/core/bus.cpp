#include "core/z80Processor.h"
#include <core/bus.h>
#include <algorithm>

using GBEmulator::Bus;

Bus::Bus()
{
    // 16kB video ram
    // Will be limited to 8kB in GB mode
    m_VRAM.resize(0x4000);

    // 32kB work ram (in GBC mode). Will be limited to 
    // 8kB in GB mode.
    m_WRAM.resize(0x8000);

    // TODO: Fill the rom with the right data
    m_ROM.fill(0x00);

    Reset();
}

void Bus::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;
        
    m_cpu.SerializeTo(visitor);
    m_cartridge->SerializeTo(visitor);

    visitor.WriteContainer(m_VRAM);
    visitor.WriteValue(m_currentVRAMBank);
    visitor.WriteContainer(m_WRAM);
    visitor.WriteValue(m_currentWRAMBank);
}

void Bus::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;

    m_cpu.DeserializeFrom(visitor);
    m_cartridge->DeserializeFrom(visitor);

    visitor.ReadContainer(m_VRAM);
    visitor.ReadValue(m_currentVRAMBank);
    visitor.ReadContainer(m_WRAM);
    visitor.ReadValue(m_currentWRAMBank);
}

void Bus::Reset()
{
    m_cpu.Reset();

    if (m_cartridge)
        m_cartridge->Reset();

    // We set all the RAM to 0 but it could be nice to
    // fill it with random values, as it often appears to be the case when
    // you boot a Gameboy
    std::fill(m_WRAM.begin(), m_WRAM.end(), 0x00);
    std::fill(m_VRAM.begin(), m_VRAM.end(), 0x00);

    // By default, we point on the first VRAM bank (won't move in GB mode)
    // Each VRAM bank is 8kB size
    m_currentVRAMBank = 0;

    // By default, we point on the second WRAM bank (won't move in GB mode)
    // Each WRAM bank is 4kB in size and two banks can be "mapped" at the same time.
    // The first one (always) and another one (switchable in GBC mode, fixed to second bank in GB mode)
    m_currentWRAMBank = 1;
}

void Bus::InsertCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
    m_cartridge = cartridge;
    Reset();
}