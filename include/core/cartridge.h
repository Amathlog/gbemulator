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

        void Reset();

    private:
        std::vector<uint8_t> m_externalRAM;
        uint8_t m_currentExternalRAMBank = 0;
    };
}