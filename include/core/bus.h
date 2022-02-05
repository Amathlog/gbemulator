#pragma once

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

        // Read a single byte of data
        uint8_t ReadByte(uint16_t addr);

        // Write a single byte of data
        void WriteByte(uint16_t addr, uint8_t data);

        void InsertCartridge(const std::shared_ptr<Cartridge>& cartridge);

    private:
        Z80Processor m_cpu;
        Mode m_mode;

        std::vector<uint8_t> m_VRAM;
        uint8_t m_currentVRAMBank;

        std::vector<uint8_t> m_WRAM;
        uint8_t m_currentWRAMBank;

        std::array<uint8_t, 256> m_ROM;

        std::shared_ptr<Cartridge> m_cartridge;
    };
}