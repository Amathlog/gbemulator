#include <algorithm>
#include <core/bus.h>
#include <core/utils/utils.h>
#include <core/z80Processor.h>
#include <cstdint>

using GBEmulator::Bus;

constexpr bool enableLogger = false;

Bus::Bus()
{
    // 32kB work ram (in GBC mode). Will be limited to
    // 8kB in GB mode.
    m_WRAM.resize(0x8000);

    // TODO: Fill the rom with the right data
    m_ROM.fill(0x00);

    // Connect to the cpu and ppu
    m_cpu.ConnectBus(this);
    m_ppu.ConnectBus(this);

    m_instLogger = std::make_unique<GBEmulator::InstructionLogger>(*this);

    Reset();
}

uint8_t Bus::ReadByte(uint16_t addr) const { return const_cast<Bus*>(this)->ReadByte(addr, true); }

uint8_t Bus::ReadByte(uint16_t addr, bool readOnly)
{
    uint8_t data = 0;

    // VRAM zone
    if (addr >= 0x8000 && addr < 0xA000)
    {
        data = m_ppu.ReadByte(addr, readOnly);
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
        data = m_ppu.ReadByte(addr);
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
        data = m_controller ? m_controller->ReadData() : 0xFF;
    }
    else if (addr == 0xFF01 || addr == 0xFF02)
    {
        // Communication
        // TODO
    }
    else if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        // Divider and timer
        data = m_timer.ReadByte(addr, readOnly);
    }
    else if (addr == IF_REG_ADDR)
    {
        // IF - Interupt flag
        data = m_IF.flag;
    }
    else if (addr >= 0xFF10 && addr <= 0xFF3F)
    {
        // Sound and Waveform RAM
        data = m_apu.ReadByte(addr);
    }
    else if (addr == 0xFF46)
    {
        // DMA
        // Write only
    }
    else if (addr >= 0xFF40 && addr <= 0xFF4B)
    {
        // LCD
        data = m_ppu.ReadByte(addr, readOnly);
    }
    else if (addr == 0xFF4D)
    {
        if (m_mode == Mode::GBC)
        {
            // Switching speed (GBC only)
            if (m_isDoubleSpeedMode)
            {
                data |= 0x80;
            }

            if (m_isPreparingForChangingSpeed)
            {
                data |= 0x01;
            }
        }
        else
        {
            data = 0xff;
        }
    }
    else if (addr == 0xFF4F && m_mode == Mode::GBC)
    {
        // VRAM bank select (GBC only)
        data = m_ppu.ReadByte(addr, readOnly);
    }
    else if (addr == 0xFF50)
    {
        // Set to non-zero to disable boot ROM
        // Not used in Read
    }
    else if (addr == 0xFF55 && m_mode == Mode::GBC)
    {
        // VRAM DMA (GBC only)
        // Only 0xFF55 is readable
        // 0xFF means the transfer is done. Otherwise, returns the number of blocks
        // remaining.
        // If DMA was stopped, write 1 to bit 7
        data = m_DMABlocksRemainingGBC == 0 ? 0xFF : m_DMABlocksRemainingGBC;
        if (m_DMAWasStoppedGBC && m_DMABlocksRemainingGBC != 0)
            data = (data | 0x80);
    }
    else if (addr >= 0xFF68 && addr <= 0xFF6B && m_mode == Mode::GBC)
    {
        // BG/OBJ palettes (GBC only)
        data = m_ppu.ReadByte(addr, readOnly);
    }
    else if (addr == 0xFF70 && m_mode == Mode::GBC)
    {
        // WRAM Bank select (GBC only)
        data = m_currentWRAMBank;
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
    // Try to read from the cartridge, if it returns true, it's done
    else if (m_cartridge && m_cartridge->ReadByte(addr, data, readOnly))
    {
        // Nothing to do
    }

    return data;
}

void Bus::WriteByte(uint16_t addr, uint8_t data)
{
    // VRAM zone
    if (addr >= 0x8000 && addr < 0xA000)
    {
        m_ppu.WriteByte(addr, data);
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
        m_ppu.WriteByte(addr, data);
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
        if (m_controller)
            m_controller->WriteData(data);
    }
    else if (addr == 0xFF01 || addr == 0xFF02)
    {
        // Communication
        // TODO
    }
    else if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        // Divider and timer
        m_timer.WriteByte(addr, data);
    }
    else if (addr == IF_REG_ADDR)
    {
        // IF - Interupt flag
        m_IF.flag = data & 0x1F;
    }
    else if (addr >= 0xFF10 && addr <= 0xFF3F)
    {
        // Sound and Waveform RAM
        m_apu.WriteByte(addr, data);
    }
    else if (addr == 0xFF46)
    {
        // DMA
        m_isInDMA = true;
        // Address can't be higher than 0xE000, so force it there (even if it shouldn't happen)
        m_currentDMAAddress = (uint16_t)(data % 0xE0) << 8;
    }
    else if (addr >= 0xFF40 && addr <= 0xFF4B)
    {
        // LCD
        m_ppu.WriteByte(addr, data);
    }
    else if (addr == 0xFF4D && m_mode == Mode::GBC)
    {
        // Switching speed (GBC Only)
        if (!!(data & 0x01))
        {
            m_isPreparingForChangingSpeed = true;
        }
    }
    else if (addr == 0xFF4F && m_mode == Mode::GBC)
    {
        // VRAM bank select (GBC only)
        m_ppu.WriteByte(addr, data);
    }
    else if (addr == 0xFF50)
    {
        // Set to non-zero to disable boot ROM
        // TODO
    }
    else if (addr >= 0xFF51 && addr <= 0xFF55 && m_mode == Mode::GBC)
    {
        // VRAM DMA (GBC only)
        switch (addr)
        {
        case 0xFF51:
            // High address source
            m_DMASourceAddrGBC = ((uint16_t)data << 8) | (m_DMASourceAddrGBC & 0x00FF);
            break;
        case 0xFF52:
            // Low address source, 4 lower bits ignored
            m_DMASourceAddrGBC = (m_DMASourceAddrGBC & 0xFF00) | (data & 0xF0);
            break;
        case 0xFF53:
            // High address dest, 3 higher bits ignored. Will be between 0x8000 and 0x9FF0
            m_DMADestAddrGBC = 0x8000 | (((uint16_t)data << 8) & 0x1F) | (m_DMADestAddrGBC & 0x00FF);
            break;
        case 0xFF54:
            // Low address dest, 4 lower bits ignored
            m_DMADestAddrGBC = (m_DMADestAddrGBC & 0xFF00) | (data & 0xF0);
            break;
        case 0xFF55:
            // If DMA is not active (no remaining blocks) writing to this starts a new DMA
            if (m_DMABlocksRemainingGBC == 0)
            {
                m_isDMAHBlankModeGBC = (data & 0x80) > 0;
                m_DMABlocksRemainingGBC = (data & 0x7F);
                m_DMAWasStoppedGBC = false;
            }
            else
            {
                // Otherwise, if we are in HBlank mode, and bit 7 is 0, stop the transfer
                m_DMAWasStoppedGBC = ((data & 0x80) == 0) && m_isDMAHBlankModeGBC;
            }
        }
    }
    else if (addr >= 0xFF68 && addr <= 0xFF6B && m_mode == Mode::GBC)
    {
        // BG/OBJ palettes (GBC only)
        m_ppu.WriteByte(addr, data);
    }
    else if (addr == 0xFF70 && m_mode == Mode::GBC)
    {
        // WRAM Bank select (GBC only)
        // A bank of 0 will select bank 1
        m_currentWRAMBank = (data & 0x07);
        if (m_currentWRAMBank == 0)
            m_currentWRAMBank = 1;
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
    // try to write from to cartridge, if it returns true, it's done
    else if (m_cartridge && m_cartridge->WriteByte(addr, data))
    {
        // Nothing to do
    }
}

// Return true if the PPU finished a frame during the clock.
bool Bus::Clock(bool* outInstDone)
{
    // No cartridge mean nothing to do
    if (!m_cartridge)
        return false;

    if (m_nbRemainingCyclesForChangingSpeed > 0 && --m_nbRemainingCyclesForChangingSpeed == 0)
    {
        m_cpu.ForceUnpause();
    }

    bool frameFinished = false;

    const unsigned numberOfPPUClocks = m_isDoubleSpeedMode ? 2 : 4;

    // Clock the PPU 4 times in single speed, 2 times in double speed.
    for (auto i = 0; i < numberOfPPUClocks; ++i)
    {
        m_ppu.Clock();
        frameFinished |= m_ppu.IsFrameComplete();
    }

    // APU is clocked every cpu cycle in single speed,
    // and every 2 cycles in double speed.
    if (!m_isDoubleSpeedMode || (m_nbCycles & 0x1) == 0)
    {
        m_apu.Clock();
    }

    // Update the controller and the interrupt
    if (m_controller)
    {
        if (m_controller->HasChangedFromHighToLow())
        {
            m_IF.joypad = 1;
        }
        m_controller->Update();
    }

    // Clock the timer always 4 times
    // So in double speed mode it will be twice as fast
    for (auto i = 0; i < 4; ++i)
    {
        if (m_timer.Clock())
        {
            m_IF.timer = 1;
        }
    }

    // If we are in DMA, copy data. CPU is still clocked
    if (m_isInDMA)
    {
        uint16_t destAddress = 0xFE00 | (m_currentDMAAddress & 0x00FF);
        WriteByte(destAddress, ReadByte(m_currentDMAAddress));
        m_currentDMAAddress++;
        if ((m_currentDMAAddress & 0x00FF) == 0xE0)
        {
            m_isInDMA = false;
        }
    }

    // But in VRAM DMA (only GBC), the CPU is stopped during the transfer
    // Transfer everything (when in general purpose mode) or a single block (in hblank mode)
    // in one clock.
    if (m_DMABlocksRemainingGBC != 0 && !m_DMAHBlankWasHandled && !m_DMAWasStoppedGBC)
    {
        if (!m_isDMAHBlankModeGBC || m_ppu.IsInHBlank())
        {
            const uint16_t nbBytesToTransfer = (uint16_t)(m_isDMAHBlankModeGBC ? 1 : m_DMABlocksRemainingGBC) * 8;
            for (uint16_t i = 0; i < nbBytesToTransfer; ++i)
            {
                WriteByte(m_DMADestAddrGBC++, ReadByte(m_DMASourceAddrGBC++));
            }

            if (m_isDMAHBlankModeGBC)
            {
                m_DMABlocksRemainingGBC--;
                m_DMAHBlankWasHandled = true;
            }
            else
            {
                m_DMABlocksRemainingGBC = 0;
            }
        }
    }

    // When we exit HBlank, reset this flag
    if (!m_ppu.IsInHBlank() && m_DMAHBlankWasHandled)
    {
        m_DMAHBlankWasHandled = false;
    }

    if (m_cpu.Clock())
    {
        if (outInstDone != nullptr)
            *outInstDone = true;

        m_instLogger->WriteCurrentState();
    }

    // Check if we need to change speed
    if (m_mode == Mode::GBC && m_isPreparingForChangingSpeed && m_cpu.IsStopped())
    {
        m_isPreparingForChangingSpeed = false;
        m_isDoubleSpeedMode = !m_isDoubleSpeedMode;
        m_nbRemainingCyclesForChangingSpeed = 2050; // Taken from PanDocs
    }

    if (m_runToAddress != 0xFFFFFFFF && (uint32_t)m_cpu.GetPC() == m_runToAddress)
    {
        m_runToAddress = 0xFFFFFFFF;
        m_isInBreakMode = true;
    }

    m_nbCycles++;

    return frameFinished;
}

void Bus::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;

    m_cpu.SerializeTo(visitor);
    m_ppu.SerializeTo(visitor);
    m_apu.SerializeTo(visitor);
    m_cartridge->SerializeTo(visitor);

    visitor.WriteValue(m_mode);

    visitor.WriteContainer(m_WRAM);
    visitor.WriteValue(m_currentWRAMBank);
    visitor.WriteValue(m_nbCycles);

    visitor.WriteContainer(m_HRAM);
    visitor.WriteValue(m_IE.flag);
    visitor.WriteValue(m_IF.flag);

    m_timer.SerializeTo(visitor);

    visitor.WriteValue(m_isInDMA);
    visitor.WriteValue(m_currentDMAAddress);

    visitor.WriteValue(m_isPreparingForChangingSpeed);
    visitor.WriteValue(m_nbRemainingCyclesForChangingSpeed);
    visitor.WriteValue(m_isDoubleSpeedMode);

    visitor.WriteValue(m_DMASourceAddrGBC);
    visitor.WriteValue(m_DMADestAddrGBC);
    visitor.WriteValue(m_DMABlocksRemainingGBC);
    visitor.WriteValue(m_isDMAHBlankModeGBC);
    visitor.WriteValue(m_DMAWasStoppedGBC);
    visitor.WriteValue(m_DMAHBlankWasHandled);
}

void Bus::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    // If we have no cartridge, nothing to do
    if (!m_cartridge)
        return;

    m_cpu.DeserializeFrom(visitor);
    m_ppu.DeserializeFrom(visitor);
    m_apu.DeserializeFrom(visitor);
    m_cartridge->DeserializeFrom(visitor);

    visitor.ReadValue(m_mode);

    visitor.ReadContainer(m_WRAM);
    visitor.ReadValue(m_currentWRAMBank);
    visitor.ReadValue(m_nbCycles);

    visitor.ReadContainer(m_HRAM);
    visitor.ReadValue(m_IE.flag);
    visitor.ReadValue(m_IF.flag);

    m_timer.DeserializeFrom(visitor);

    visitor.ReadValue(m_isInDMA);
    visitor.ReadValue(m_currentDMAAddress);

    visitor.ReadValue(m_isPreparingForChangingSpeed);
    visitor.ReadValue(m_nbRemainingCyclesForChangingSpeed);
    visitor.ReadValue(m_isDoubleSpeedMode);

    visitor.ReadValue(m_DMASourceAddrGBC);
    visitor.ReadValue(m_DMADestAddrGBC);
    visitor.ReadValue(m_DMABlocksRemainingGBC);
    visitor.ReadValue(m_isDMAHBlankModeGBC);
    visitor.ReadValue(m_DMAWasStoppedGBC);
    visitor.ReadValue(m_DMAHBlankWasHandled);
}

void Bus::Reset()
{
    m_cpu.Reset();
    m_ppu.Reset();
    m_apu.Reset();

    if (m_cartridge)
        m_cartridge->Reset();

    // We set all the RAM to 0 but it could be nice to
    // fill it with random values, as it often appears to be the case when
    // you boot a Gameboy
    std::fill(m_WRAM.begin(), m_WRAM.end(), 0x00);
    m_HRAM.fill(0x00);

    // By default, we point on the second WRAM bank (won't move in GB mode)
    // Each WRAM bank is 4kB in size and two banks can be "mapped" at the same time.
    // The first one (always) and another one (switchable in GBC mode, fixed to second bank in GB mode)
    m_currentWRAMBank = 1;

    m_IE.flag = 0x00;
    m_IF.flag = 0x00;

    if (m_controller)
    {
        m_controller->Reset();
    }

    m_nbCycles = 0;

    m_timer.Reset();

    if (enableLogger)
        m_instLogger->OpenFileVisitor("dump.txt");

    m_isInDMA = false;
    m_currentDMAAddress = 0x0000;

    m_DMASourceAddrGBC = 0x0000;
    m_DMADestAddrGBC = 0x8000;
    m_DMABlocksRemainingGBC = 0x00;
    m_isDMAHBlankModeGBC = false;
    m_DMAWasStoppedGBC = false;
    m_DMAHBlankWasHandled = false;
}

void Bus::ChangeMode(Mode newMode)
{
    if ((newMode == Mode::GB && !IsGBModeAvailable()) || (newMode == Mode::GBC && !IsGBCModeAvailable()))
        return;

    m_mode = newMode;

    // No need to reset if we have no game loaded.
    if (m_cartridge)
        Reset();
}

bool Bus::IsGBModeAvailable() const
{
    // Return true if we have no cartridge to allow the user to change it
    // before starting a game.
    return m_cartridge ? !m_cartridge->GetHeader().CGBOnly : true;
}

bool Bus::IsGBCModeAvailable() const
{
    // Return true if we have no cartridge to allow the user to change it
    // before starting a game.
    return m_cartridge ? (m_cartridge->GetHeader().CGBOnly || m_cartridge->GetHeader().supportCGBMode) : true;
}

void Bus::InsertCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
    if (!cartridge)
        return;

    m_cartridge = cartridge;

    // If the game supports both modes, let it. Otherwise, set it to the supported mode.
    if (!IsGBModeAvailable())
    {
        m_mode = Mode::GBC;
    }
    else if (!IsGBCModeAvailable())
    {
        m_mode = Mode::GB;
    }

    Reset();

    m_isInBreakMode = m_shouldBreakOnStart;
}

void Bus::ConnectController(const std::shared_ptr<Controller>& controller)
{
    m_controller = controller;
    m_controller->Reset();
}

void Bus::SetRunToAddress(uint16_t address)
{
    m_runToAddress = address;
    m_isInBreakMode = false;
}
