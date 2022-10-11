#pragma once

#include "core/utils/visitor.h"
#include <core/serializable.h>
#include <core/z80Processor.h>
#include <core/2C02Processor.h>
#include <core/cartridge.h>
#include <core/controller.h>
#include <core/timer.h>
#include <core/apu.h>
#include <core/utils/instLogger.h>
#include <vector>
#include <array>
#include <cstdint>
#include <memory>

namespace GBEmulator 
{
    enum class Mode
    {
        GB,
        GBC
    };

    union InterruptRegister
    {
        struct
        {
            uint8_t vBlank : 1;
            uint8_t lcdStat : 1;
            uint8_t timer : 1;
            uint8_t serial : 1;
            uint8_t joypad : 1;
            uint8_t unused : 3;
        };

        uint8_t flag = 0x00;
    };

    class Bus : public ISerializable
    {
    public:
        // Allow the CPU to access variables directly
        friend Z80Processor;

        Bus();

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();

        bool IsInBreak() const { return m_isInBreakMode; }
        void BreakContinue() { m_isInBreakMode = !m_isInBreakMode; }
        void SetRunToAddress(uint16_t address);
        void SetBreakOnStart(bool value) { m_shouldBreakOnStart = value; }

        // Clock a CPU cycle
        bool Clock(bool* outInstDone = nullptr);

        // Read a single byte of data
        // Use the const version to read it or use readOnly flag to avoid alter the memory
        // (ie. some operations can "write" data while reading)
        uint8_t ReadByte(uint16_t addr, bool readOnly = false);
        uint8_t ReadByte(uint16_t addr) const;

        // Write a single byte of data
        void WriteByte(uint16_t addr, uint8_t data);

        void InsertCartridge(const std::shared_ptr<Cartridge>& cartridge);
        void ConnectController(const std::shared_ptr<Controller>& controller);

        const Cartridge* GetCartridge() const { return m_cartridge.get(); }
        const Z80Processor& GetCPU() const { return m_cpu; }
        const Processor2C02& GetPPU() const { return m_ppu; }
        APU& GetAPU() { return m_apu; }
        void SetPC(uint16_t addr) { m_cpu.SetPC(addr); }

        void SaveCartridgeRAM(Utils::IWriteVisitor& visitor) const { m_cartridge->SerializeRam(visitor); }
        void LoadCartridgeRAM(Utils::IReadVisitor& visitor) { m_cartridge->DeserializeRam(visitor); }

        // Change mode if possible.
        // Will reset the game
        void ChangeMode(Mode newMode);
        Mode GetMode() const { return m_mode; }
        bool IsGBModeAvailable() const;
        bool IsGBCModeAvailable() const;

        bool IsInDoubleSpeedMode() const { return m_isDoubleSpeedMode; }

    private:        
        Z80Processor m_cpu;
        Processor2C02 m_ppu;
        APU m_apu;
        Timer m_timer;
        std::unique_ptr<InstructionLogger> m_instLogger;

        Mode m_mode = Mode::GB;

        bool m_isPreparingForChangingSpeed = false;
        uint16_t m_nbRemainingCyclesForChangingSpeed = 0x0000;
        bool m_isDoubleSpeedMode = false;

        std::vector<uint8_t> m_WRAM;
        uint8_t m_currentWRAMBank;

        std::array<uint8_t, 127> m_HRAM;

        InterruptRegister m_IE;
        InterruptRegister m_IF;

        std::array<uint8_t, 256> m_ROM;

        std::shared_ptr<Cartridge> m_cartridge;
        std::shared_ptr<Controller> m_controller;
        size_t m_nbCycles;

        bool m_isInBreakMode = false;
        bool m_shouldBreakOnStart = false;
        uint32_t m_runToAddress = 0xFFFFFFFF;

        // DMA
        bool m_isInDMA = false;
        uint16_t m_currentDMAAddress = 0x0000;

        // GBC DMA
        uint16_t m_DMASourceAddrGBC = 0x0000;
        uint16_t m_DMADestAddrGBC = 0x0000;
        uint8_t m_DMABlocksRemainingGBC = 0x00;
        bool m_isDMAHBlankModeGBC = false;
        bool m_DMAHBlankWasHandled = false;
        bool m_DMAWasStoppedGBC = false;
    };
}