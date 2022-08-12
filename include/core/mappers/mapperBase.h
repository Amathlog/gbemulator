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

        bool HasCustomReadWrite(uint16_t addr) const
        {
            return addr >= m_startAddrCustomRead && addr < m_endAddrCustomRead;
        }

        // Returns true if it handled the call
        virtual bool ReadByte(uint16_t addr, uint8_t& data) const { return false; }

        void SerializeTo(Utils::IWriteVisitor& visitor) const override
        {
            visitor.WriteValue(m_ramEnabled);
            visitor.WriteValue(m_firstRomBank);
            visitor.WriteValue(m_secondRomBank);
            visitor.WriteValue(m_ramBank);
            visitor.WriteValue(m_startAddrCustomRead);
            visitor.WriteValue(m_endAddrCustomRead);
        }

        void DeserializeFrom(Utils::IReadVisitor& visitor) override
        {
            visitor.ReadValue(m_ramEnabled);
            visitor.ReadValue(m_firstRomBank);
            visitor.ReadValue(m_secondRomBank);
            visitor.ReadValue(m_ramBank);
            visitor.ReadValue(m_startAddrCustomRead);
            visitor.ReadValue(m_endAddrCustomRead);
        }

        virtual void Reset()
        {
            m_ramEnabled = false;
            m_firstRomBank = 0x0000;
            m_secondRomBank = 0x0001;
            m_ramBank = 0x00;
            m_startAddrCustomRead = 0x0000;
            m_endAddrCustomRead = 0x0000;
        }

        inline bool IsRamEnabled() const { return m_ramEnabled; }
        inline uint16_t GetFirstROMBank() const { return m_firstRomBank; }
        inline uint16_t GetSecondROMBank() const { return m_secondRomBank; }
        inline uint8_t GetRAMBank() const { return m_ramBank; }

        // Specific for MBC2 
        virtual size_t GetRAMSize() const { return m_header.nbRamBanks * 0x2000; }

    protected:
        const Header& m_header; 
        bool m_ramEnabled = false;
        uint16_t m_firstRomBank = 0x0000;
        uint16_t m_secondRomBank = 0x0001;
        uint8_t m_ramBank = 0x00;

        uint16_t m_startAddrCustomRead = 0x0000;
        uint16_t m_endAddrCustomRead = 0x0000;
    };
}