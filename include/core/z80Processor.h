#pragma once

#include <core/utils/visitor.h>
#include <core/serializable.h>
#include <cstdint>
#include <type_traits>
#include <array>
#include <string>

namespace GBEmulator
{
    class Bus;

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
        Z80Processor();

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();
        bool Clock();

        uint8_t ReadByte(uint16_t addr);
        void WriteByte(uint16_t addr, uint8_t data);

        RegisterAF GetAFRegister() const { return m_AF; } 
        RegisterBC GetBCRegister() const { return m_BC; } 
        RegisterDE GetDERegister() const { return m_DE; } 
        RegisterHL GetHLRegister() const { return m_HL; }

        uint16_t GetStackPointer() const { return m_SP; }
        uint16_t GetPrgCounter() const { return m_PC; }

        void ConnectBus(Bus* bus) { m_bus = bus; }

    private:
        // Utility functions
        void SetZeroFlag(uint16_t res) { m_AF.F.Z = (res == 0); }
        // Will get/write the value of the register with a given index in data
        // Return true if the data was read from/write to memory (index 6)
        // indicating that we have an additional cycle
        bool ReadByteFromRegisterIndex(uint8_t index, uint8_t& data);
        bool WriteByteToRegisterIndex(uint8_t index, uint8_t data);

        // Same than previous, but for words (u16)
        // It can be register BC, DE and HF, and the stack pointer (SP)
        // Since we can't fetch words in memory, only byte, we drop the bool
        // return value (always false)
        void ReadWordFromRegisterIndex(uint8_t index, uint16_t& data);
        void WriteWordToRegisterIndex(uint8_t index, uint16_t data);

        // Return the number of cycles required by this opcode
        uint8_t DecodeOpcodeAndCall(uint8_t opcode);

        // Fetch data from program
        uint8_t FetchByte();
        uint16_t FetchWord();

        // Operate the stack
        void PushWordToStack(uint16_t data);
        uint16_t PopWordFromStack();

        // Declaration of all "types" of opcodes
        // We also pass the opcode to the function as
        // it contains information like the register to read from/write to...
        // Will return the number of cycles required
        #define DECLARE_OP(OP) uint8_t OP(uint8_t opcode)

        // Load operations
        DECLARE_OP(LD);   DECLARE_OP(LDH);

        // Arithmetic/Logic instructions
        DECLARE_OP(ADD);  DECLARE_OP(ADC);  DECLARE_OP(SUB);
        DECLARE_OP(SBC);  DECLARE_OP(AND);  DECLARE_OP(OR);
        DECLARE_OP(XOR);  DECLARE_OP(CP);   DECLARE_OP(DEC);
        DECLARE_OP(INC);

        // Bit operations instructions
        DECLARE_OP(BIT);  DECLARE_OP(RES);  DECLARE_OP(SET);
        DECLARE_OP(SWAP);

        // Bit shift instructions
        DECLARE_OP(RL);   DECLARE_OP(RLA);  DECLARE_OP(RLC);
        DECLARE_OP(RLCA); DECLARE_OP(RR);   DECLARE_OP(RRA);
        DECLARE_OP(RRC);  DECLARE_OP(RRCA); DECLARE_OP(SLA);
        DECLARE_OP(SRA);  DECLARE_OP(SRL);

        // Dispatcher instruction
        DECLARE_OP(DISP);

        // Jumps and Subroutines
        DECLARE_OP(CALL); DECLARE_OP(JP);   DECLARE_OP(JR);
        DECLARE_OP(RET);  DECLARE_OP(RETI); DECLARE_OP(RST);

        // Stack operations
        DECLARE_OP(POP);  DECLARE_OP(PUSH); DECLARE_OP(LDSP);

        // Misc instructions
        DECLARE_OP(CCF);  DECLARE_OP(CPL);  DECLARE_OP(DAA);
        DECLARE_OP(DI);   DECLARE_OP(EI);   DECLARE_OP(HALT);
        DECLARE_OP(NOP);  DECLARE_OP(SCF);  DECLARE_OP(STOP);

        // Invalid instruction
        DECLARE_OP(XXX);

        #undef DECLARE_OP

        // Registers
        RegisterAF m_AF;
        RegisterBC m_BC;
        RegisterDE m_DE;
        RegisterHL m_HL;

        uint16_t m_SP;
        uint16_t m_PC;

        uint8_t m_cycles = 0;

        // IME flags
        bool m_IMEScheduled = false;
        bool m_IMEEnabled = false;

        // Other members
        Bus* m_bus;

        using OpCall = uint8_t(Z80Processor::*)(uint8_t);
        using Z = Z80Processor;

        static constexpr std::array<OpCall, 256> m_opcodesMap =
        {
            &Z::NOP,  &Z::LD,   &Z::LD,   &Z::INC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::RLCA, &Z::LD,   &Z::ADD,  &Z::LD,   &Z::DEC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::RRCA, 
            &Z::STOP, &Z::LD,   &Z::LD,   &Z::INC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::RLA,  &Z::JR,   &Z::ADD,  &Z::LD,   &Z::DEC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::RRA, 
            &Z::JR,   &Z::LD,   &Z::LD,   &Z::INC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::DAA,  &Z::JR,   &Z::ADD,  &Z::LD,   &Z::DEC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::CPL, 
            &Z::JR,   &Z::LD,   &Z::LD,   &Z::INC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::SCF,  &Z::JR,   &Z::ADD,  &Z::LD,   &Z::DEC,  &Z::INC,  &Z::DEC,  &Z::LD,   &Z::CCF, 
            &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD, 
            &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD, 
            &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD, 
            &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::HALT, &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD,   &Z::LD, 
            &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADD,  &Z::ADC,  &Z::ADC,  &Z::ADC,  &Z::ADC,  &Z::ADC,  &Z::ADC,  &Z::ADC,  &Z::ADC, 
            &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SUB,  &Z::SBC,  &Z::SBC,  &Z::SBC,  &Z::SBC,  &Z::SBC,  &Z::SBC,  &Z::SBC,  &Z::SBC, 
            &Z::AND,  &Z::AND,  &Z::AND,  &Z::AND,  &Z::AND,  &Z::AND,  &Z::AND,  &Z::AND,  &Z::XOR,  &Z::XOR,  &Z::XOR,  &Z::XOR,  &Z::XOR,  &Z::XOR,  &Z::XOR,  &Z::XOR, 
            &Z::OR,   &Z::OR,   &Z::OR,   &Z::OR,   &Z::OR,   &Z::OR,   &Z::OR,   &Z::OR,   &Z::CP,   &Z::CP,   &Z::CP,   &Z::CP,   &Z::CP,   &Z::CP,   &Z::CP,   &Z::CP,  
            &Z::RET,  &Z::POP,  &Z::JP,   &Z::JP,   &Z::CALL, &Z::PUSH, &Z::ADD,  &Z::RST,  &Z::RET,  &Z::RET,  &Z::JP,   &Z::DISP, &Z::CALL, &Z::CALL, &Z::ADC,  &Z::RST, 
            &Z::RET,  &Z::POP,  &Z::JP,   &Z::XXX,  &Z::CALL, &Z::PUSH, &Z::SUB,  &Z::RST,  &Z::RET,  &Z::RETI, &Z::JP,   &Z::XXX,  &Z::CALL, &Z::XXX,  &Z::SBC,  &Z::RST, 
            &Z::LDH,  &Z::POP,  &Z::LD,   &Z::XXX,  &Z::XXX,  &Z::PUSH, &Z::AND,  &Z::RST,  &Z::ADD,  &Z::JP,   &Z::LD,   &Z::XXX,  &Z::XXX,  &Z::XXX,  &Z::XOR,  &Z::RST, 
            &Z::LDH,  &Z::POP,  &Z::LD,   &Z::DI,   &Z::XXX,  &Z::PUSH, &Z::OR,   &Z::RST,  &Z::LDSP, &Z::LD,   &Z::LD,   &Z::EI,   &Z::XXX,  &Z::XXX,  &Z::CP,   &Z::RST, 
        };

        constexpr const char* GetDebugStringForOp(uint8_t opcode);
    };
}