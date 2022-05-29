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

    // Connect to the cpu and ppu
    m_cpu.ConnectBus(this);
    m_ppu.ConnectBus(this);

    Reset();
}

uint8_t Bus::ReadByte(uint16_t addr)
{
    uint8_t data = 0;

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
    else if (addr >= 0xC000 && addr <= 0xFDFF)
    {
        // WRAM is between 0xC000 and 0xDFFF, but zone between 0xE000 and 0xFDFF, even if it should
        // not be used, will echo the RAM (0xC000 -> 0xDDFF)
        if (addr >= 0xE000)
        {
            addr -= 0x2000;
        }

        // Get the WRAM bank
        // Between 0xC000 and 0xCFFF it's bank 0
        // Between 0xD000 and 0xDFFF it's bank switchable
        uint8_t wramBank = (addr & 0x1000) ? m_currentWRAMBank : 0;
        // WRAM banks are 4kB in size
        data = m_WRAM[wramBank * 0x1000 + (addr & 0x0FFF)];
    }
    // Sprite attribute table (OAM)
    else if (addr >= 0xFE00 && addr <= 0xFE9F)
    {
        // TODO
    }
    // Special RAM space
    else if (addr >= 0xFEA0 && addr <= 0xFEFF)
    {
        // TODO
    }
    // Misc zone
    else if (addr == 0xFF00)
    {
        // Controller
        // TODO
    }
    else if (addr == 0xFF01 || addr == 0xFF02)
    {
        // Communication
        // TODO
    }
    else if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        // Divider and timer
        // TODO
    }
    else if (addr == IF_REG_ADDR)
    {
        // IF - Interupt flag
        data = m_IF.flag;
    }
    else if (addr >= 0xFF10 && addr <= 0xFF26)
    {
        // Sound
        // TODO
    }
    else if (addr >= 0xFF30 && addr <= 0xFF3F)
    {
        // Waveform RAM
        // TODO
    }
    else if (addr >= 0xFF40 && addr <= 0xFF4B)
    {
        // LCD
        data = m_ppu.ReadByte(addr);
    }
    else if (addr == 0xFF4F && m_mode == Mode::GBC)
    {
        // VRAM bank select (GBC only)
        // TODO
    }
    else if (addr == 0xFF50)
    {
        // Set to non-zero to disable boot ROM
        // Not used in Read
    }
    else if (addr >= 0xFF51 && addr <= 0xFF55 && m_mode == Mode::GBC)
    {
        // VRAM DMA (GBC only)
        // TODO
    }
    else if (addr >= 0xFF68 && addr <= 0xFF6B && m_mode == Mode::GBC)
    {
        // BG/OBJ palettes (GBC only)
        data = m_ppu.ReadByte(addr);
    }
    else if (addr == 0xFF70 && m_mode == Mode::GBC)
    {
        // WRAM Bank select (GBC only)
        // TODO
    }
    else if (addr >= 0xFF80 && addr <= 0xFFFE)
    {
        // High RAM
        data = m_HRAM[addr - 0xFF80];
    }
    else if (addr == IE_REG_ADDR)
    {
        // Interupt Enable Register (IE)
        data = m_IE.flag;
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
    else if (addr >= 0xC000 && addr <= 0xFDFF)
    {
        // WRAM is between 0xC000 and 0xDFFF, but zone between 0xE000 and 0xFDFF, even if it should
        // not be used, will echo the RAM (0xC000 -> 0xDDFF)
        if (addr >= 0xE000)
        {
            addr -= 0x2000;
        }

        // Get the WRAM bank
        // Between 0xC000 and 0xCFFF it's bank 0
        // Between 0xD000 and 0xDFFF it's bank switchable
        uint8_t wramBank = (addr & 0x1000) ? m_currentWRAMBank : 0;
        // WRAM banks are 4kB in size
        m_WRAM[wramBank * 0x1000 + (addr & 0x0FFF)] = data;
    }
    // Sprite attribute table (OAM)
    else if (addr >= 0xFE00 && addr <= 0xFE9F)
    {
        // TODO
    }
    // Special RAM space
    else if (addr >= 0xFEA0 && addr <= 0xFEFF)
    {
        // TODO
    }
    // Misc zone
    else if (addr == 0xFF00)
    {
        // Controller
        // TODO
    }
    else if (addr == 0xFF01 || addr == 0xFF02)
    {
        // Communication
        // TODO
    }
    else if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        // Divider and timer
        // TODO
    }
    else if (addr == IF_REG_ADDR)
    {
        // IF - Interupt flag
        m_IF.flag = data & 0x1F;
    }
    else if (addr >= 0xFF10 && addr <= 0xFF26)
    {
        // Sound
        // TODO
    }
    else if (addr >= 0xFF30 && addr <= 0xFF3F)
    {
        // Waveform RAM
        // TODO
    }
    else if (addr >= 0xFF40 && addr <= 0xFF4B)
    {
        // LCD
        m_ppu.WriteByte(addr, data);
    }
    else if (addr == 0xFF4F && m_mode == Mode::GBC)
    {
        // VRAM bank select (GBC only)
        // TODO
    }
    else if (addr == 0xFF50)
    {
        // Set to non-zero to disable boot ROM
        // TODO
    }
    else if (addr >= 0xFF51 && addr <= 0xFF55 && m_mode == Mode::GBC)
    {
        // VRAM DMA (GBC only)
        // TODO
    }
    else if (addr >= 0xFF68 && addr <= 0xFF6B && m_mode == Mode::GBC)
    {
        // BG/OBJ palettes (GBC only)
        m_ppu.WriteByte(addr, data);
    }
    else if (addr == 0xFF70 && m_mode == Mode::GBC)
    {
        // WRAM Bank select (GBC only)
        // TODO
    }
    else if (addr >= 0xFF80 && addr <= 0xFFFE)
    {
        // High RAM
        m_HRAM[addr - 0xFF80] = data;
    }
    else if (addr == IE_REG_ADDR)
    {
        // Interupt Enable Register (IE)
        m_IE.flag = data & 0x1F;
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

        if (m_runToAddress != 0xFFFFFFFF && (uint32_t)m_cpu.GetPC() == m_runToAddress)
        {
            m_runToAddress = 0xFFFFFFFF;
            m_isInBreakMode = true;
        }
    }

    m_ppu.Clock();

    m_nbCycles++;

    return res;
}

void Bus::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;
        
    m_cpu.SerializeTo(visitor);
    m_ppu.SerializeTo(visitor);
    m_cartridge->SerializeTo(visitor);

    visitor.WriteContainer(m_VRAM);
    visitor.WriteValue(m_currentVRAMBank);
    visitor.WriteContainer(m_WRAM);
    visitor.WriteValue(m_currentWRAMBank);
    visitor.WriteValue(m_nbCycles);

    visitor.WriteContainer(m_HRAM);
    visitor.WriteValue(m_IE.flag);
    visitor.WriteValue(m_IF.flag);
}

void Bus::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;

    m_cpu.DeserializeFrom(visitor);
    m_ppu.DeserializeFrom(visitor);
    m_cartridge->DeserializeFrom(visitor);

    visitor.ReadContainer(m_VRAM);
    visitor.ReadValue(m_currentVRAMBank);
    visitor.ReadContainer(m_WRAM);
    visitor.ReadValue(m_currentWRAMBank);
    visitor.ReadValue(m_nbCycles);

    visitor.ReadContainer(m_HRAM);
    visitor.ReadValue(m_IE.flag);
    visitor.ReadValue(m_IF.flag);
}

void Bus::Reset()
{
    m_cpu.Reset();
    m_ppu.Reset();

    if (m_cartridge)
        m_cartridge->Reset();

    // We set all the RAM to 0 but it could be nice to
    // fill it with random values, as it often appears to be the case when
    // you boot a Gameboy
    std::fill(m_WRAM.begin(), m_WRAM.end(), 0x00);
    std::fill(m_VRAM.begin(), m_VRAM.end(), 0x00);
    m_HRAM.fill(0x00);

    // By default, we point on the first VRAM bank (won't move in GB mode)
    // Each VRAM bank is 8kB size
    m_currentVRAMBank = 0;

    // By default, we point on the second WRAM bank (won't move in GB mode)
    // Each WRAM bank is 4kB in size and two banks can be "mapped" at the same time.
    // The first one (always) and another one (switchable in GBC mode, fixed to second bank in GB mode)
    m_currentWRAMBank = 1;

    m_IE.flag = 0x00;
    m_IF.flag = 0x00;

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
    if (!cartridge)
        return;

    // Set the current mode to the highest supported
    // GBC not supported yet
    if (cartridge->GetHeader().CGBOnly)
        return;

    m_cartridge = cartridge;

    //m_mode = m_cartridge->GetHeader().supportCGBMode ? Mode::GBC : Mode::GB;
    m_mode = Mode::GB;

    Reset();
}

void Bus::SetRunToAddress(uint16_t address)
{
    m_runToAddress = address;
    m_isInBreakMode = false;
}
