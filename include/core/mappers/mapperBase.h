#pragma once

#include <core/serializable.h>
#include <core/cartridge.h>
#include <core/utils/visitor.h>
#include <cstdint>

namespace GBEmulator
{
    class MapperBase : public ISerializable
    {
    public:
        MapperBase(const Header& header) : m_header(header) {}

        virtual ~MapperBase() = default;

        // Returns true if it handled the call
        virtual bool WriteByte(uint16_t addr, uint8_t data) = 0;

        void SerializeTo(Utils::IWriteVisitor& visitor) const override
        {
            visitor.WriteValue(m_ramEnabled);
            visitor.WriteValue(m_firstRomBank);
            visitor.WriteValue(m_secondRomBank);
            visitor.WriteValue(m_ramBank);
        }

        void DeserializeFrom(Utils::IReadVisitor& visitor) override
        {
            visitor.ReadValue(m_ramEnabled);
            visitor.ReadValue(m_firstRomBank);
            visitor.ReadValue(m_secondRomBank);
            visitor.ReadValue(m_ramBank);
        }

        void Reset()
        {
            m_ramEnabled = false;
            m_firstRomBank = 0x00;
            m_secondRomBank = 0x01;
            m_ramBank = 0x00;
        }

        inline bool IsRamEnabled() const { return m_ramEnabled; }
        inline uint8_t GetFirstROMBank() const { return m_firstRomBank; }
        inline uint8_t GetSecondROMBank() const { return m_secondRomBank; }
        inline uint8_t GetRAMBank() const { return m_ramBank; }

    protected:
        const Header& m_header; 
        bool m_ramEnabled = false;
        uint8_t m_firstRomBank = 0x00;
        uint8_t m_secondRomBank = 0x01;
        uint8_t m_ramBank = 0x00;
    };
}