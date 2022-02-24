#pragma once

#include "core/utils/visitor.h"
#include <core/serializable.h>
#include <core/z80Processor.h>
#include <core/cartridge.h>
#include <vector>
#include <array>
#include <cstdint>
#include <memory>

namespace GBEmulator 
{
    enum Mode
    {
        GB,
        GBC
    };

    class Bus : public ISerializable
    {
    public:
        Bus();

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();
        Mode GetCurrentMode() const { return m_mode; }

        bool IsInBreak() const { return m_isInBreakMode; }
        void BreakContinue() { m_isInBreakMode = !m_isInBreakMode; }

        bool Clock();

        // Read a single byte of data
        uint8_t ReadByte(uint16_t addr);

        // Write a single byte of data
        void WriteByte(uint16_t addr, uint8_t data);

        void InsertCartridge(const std::shared_ptr<Cartridge>& cartridge);

        const Cartridge* GetCartridge() const { return m_cartridge.get(); }
        const Z80Processor& GetCPU() const { return m_cpu; }

        void SaveCartridgeRAM(Utils::IWriteVisitor& visitor) const { m_cartridge->SerializeTo(visitor); }
        void LoadCartridgeRAM(Utils::IReadVisitor& visitor) { m_cartridge->DeserializeFrom(visitor); }

        // Change mode if possible.
        // Will reset the game
        void ChangeMode(Mode newMode);
        Mode GetMode() const { return m_mode; }

        double GetCurrentFrequency() const { return m_isDoubleSpeedMode ? 8338608.0 : 4194304.0; }

    private:        
        Z80Processor m_cpu;
        Mode m_mode;

        bool m_isDoubleSpeedMode = false;

        std::vector<uint8_t> m_VRAM;
        uint8_t m_currentVRAMBank;

        std::vector<uint8_t> m_WRAM;
        uint8_t m_currentWRAMBank;

        std::array<uint8_t, 256> m_ROM;

        std::shared_ptr<Cartridge> m_cartridge;
        size_t m_nbCycles;

        bool m_isInBreakMode = false;
    };
}