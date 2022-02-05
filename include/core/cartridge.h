#pragma once

#include <core/serializable.h>
#include <vector>
#include <cstdint>

namespace GBEmulator
{
    class Cartridge : public ISerializable
    {
    public:
        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        // Read a single byte of data, passed as an out argument
        // Will return true if the cartridge has provided some data
        bool ReadByte(uint16_t addr, uint8_t& data);

        // Write a single byte of data
        // Will return true if the cartridge has done a write operation
        bool WriteByte(uint16_t addr, uint8_t data);

        void Reset();

    private:
        std::vector<uint8_t> m_externalRAM;
        uint8_t m_currentExternalRAMBank = 0;

        std::vector<uint8_t> m_prgData;
        uint8_t m_currentPrgDataBank;
    };
}