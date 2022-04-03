#include "core/z80Processor.h"
#include <core/bus.h>
#include <algorithm>
#include <cstdint>

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

    // Connect to the cpu
    m_cpu.ConnectBus(this);

    Reset();
}

uint8_t Bus::ReadByte(uint16_t addr)
{
    uint8_t data;
    // First try to read from the cartridge, if it returns true, it's done
    if (m_cartridge && m_cartridge->ReadByte(addr, data))
    {
        // Nothing to do
    }
    // VRAM zone
    else if (addr >= 0x8000 && addr < 0xA000)
    {
        // VRAM banks are 8kB in size
        data = m_VRAM[m_currentVRAMBank * 0x2000 + (addr & 0x1FFF)];
    }
    // WRAM zone
    else if (addr >= 0xC000 && addr < 0xE000)
    {
        // Get the WRAM bank
        // Between 0xC000 and 0xCFFF it's bank 0
        // Between 0xD000 and 0xDFFF it's bank switchable
        uint8_t wramBank = (addr & 0x1000) ? m_currentWRAMBank : 0;
        // WRAM banks are 4kB in size
        data = m_WRAM[wramBank * 0x1000 + (addr & 0x0FFF)];
    }
    // Misc zone
    else if (addr >= 0xE000)
    {
        // TODO
    }

    return data;
}

void Bus::WriteByte(uint16_t addr, uint8_t data)
{
    // First try to write from to cartridge, if it returns true, it's done
    if (m_cartridge && m_cartridge->WriteByte(addr, data))
    {
        // Nothing to do
    }
    // VRAM zone
    else if (addr >= 0x8000 && addr < 0xA000)
    {
        // VRAM banks are 8kB in size
        m_VRAM[m_currentVRAMBank * 0x2000 + (addr & 0x1FFF)] = data;
    }
    // WRAM zone
    else if (addr >= 0xC000 && addr < 0xE000)
    {
        // Get the WRAM bank
        // Between 0xC000 and 0xCFFF it's bank 0
        // Between 0xD000 and 0xDFFF it's bank switchable
        uint8_t wramBank = (addr & 0x1000) ? m_currentWRAMBank : 0;
        // WRAM banks are 4kB in size
        m_WRAM[wramBank * 0x1000 + (addr & 0x0FFF)] = data;
    }
    // Misc zone
    else if (addr >= 0xE000)
    {
        // TODO
        // Serial transfer data (SB)
        if (addr == 0xFF01)
        {
            // TODO
            data = data;
        }
    }
}

// Return true if it has terminated the latest instruction (or there is nothing to do)
bool Bus::Clock()
{
    // No cartridge mean nothing to do
    if (!m_cartridge)
        return true;

    bool res = false;

    // CPU is clocked every 4 ticks
    if (m_nbCycles % 4 == 0)
    {
        res = m_cpu.Clock();
    }

    m_nbCycles++;

    return res;
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
    visitor.WriteValue(m_nbCycles);
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
    visitor.ReadValue(m_nbCycles);
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

    m_nbCycles = 0;
}

void Bus::ChangeMode(Mode newMode)
{
    if (!m_cartridge)
        return;

    if (newMode == Mode::GB && m_cartridge->GetHeader().CGBOnly ||
        newMode == Mode::GBC && !m_cartridge->GetHeader().supportCGBMode)
        return;

    m_mode = newMode;
    Reset();
}

void Bus::InsertCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
    m_cartridge = cartridge;
    Reset();

    // Set the current mode to the highest supported
    m_mode = m_cartridge->GetHeader().supportCGBMode ? Mode::GBC : Mode::GB;
}