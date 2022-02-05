#pragma once

#include <core/utils/visitor.h>
#include <core/serializable.h>
#include <cstdint>

namespace GBEmulator
{
    // Registers
    union Flags
    {
        struct
        {
            uint8_t unused:4;   // Bottom 4 bits, should be 0 all the time
            uint8_t C:1;        // Carry flag
            uint8_t H:1;        // Half-Carry flag
            uint8_t N:1;        // Substraction flag
            uint8_t Z:1;        // Zero flag
        };

        uint8_t flags = 0x00;
    };

    // We use Unions to concatenate 2 registers in the same structure.
    // They can be accessed individually or by pair.
    // Because x86 system is little-endian, the low part of the 16 bit int
    // is the first in the anonymous struct.
    union RegisterAF
    {
        RegisterAF(uint16_t v = 0x0000) : AF(v) {}

        struct
        {
            Flags F;
            uint8_t A;
        };

        uint16_t AF = 0x0000;
    };

    union RegisterBC
    {
        struct
        {
            uint8_t C;
            uint8_t B;
        };

        uint16_t BC = 0x0000;
    };

    union RegisterDE
    {
        struct
        {
            uint8_t E;
            uint8_t D;
        };

        uint16_t DE = 0x0000;
    };

    union RegisterHL
    {
        struct
        {
            uint8_t L;
            uint8_t H;
        };

        uint16_t HL = 0x0000;
    };

    class Z80Processor : public ISerializable
    {
    public:
        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();

        RegisterAF GetAFRegister() const { return m_AF; } 
        RegisterBC GetBCRegister() const { return m_BC; } 
        RegisterDE GetDERegister() const { return m_DE; } 
        RegisterHL GetHLRegister() const { return m_HL; }

        uint16_t GetStackPointer() const { return m_SP; }
        uint16_t GetPrgCounter() const { return m_PC; }

    private:

        // Registers
        RegisterAF m_AF;
        RegisterBC m_BC;
        RegisterDE m_DE;
        RegisterHL m_HL;

        uint16_t m_SP;
        uint16_t m_PC;

        uint8_t m_cycles = 0;
    };
}