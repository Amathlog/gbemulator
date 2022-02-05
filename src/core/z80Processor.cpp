#include <core/z80Processor.h>
#include <core/bus.h>
#include <cstdint>

using GBEmulator::Z80Processor;

Z80Processor::Z80Processor()
{
    Reset();
} 

void Z80Processor::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_AF.AF);
    visitor.WriteValue(m_BC.BC);
    visitor.WriteValue(m_DE.DE);
    visitor.WriteValue(m_HL.HL);
    visitor.WriteValue(m_SP);
    visitor.WriteValue(m_PC);
    visitor.WriteValue(m_cycles);
}

void Z80Processor::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_AF.AF);
    visitor.ReadValue(m_BC.BC);
    visitor.ReadValue(m_DE.DE);
    visitor.ReadValue(m_HL.HL);
    visitor.ReadValue(m_SP);
    visitor.ReadValue(m_PC);
    visitor.ReadValue(m_cycles);
}

void Z80Processor::Reset()
{
    m_AF.AF = 0x0000;
    m_BC.BC = 0x0000;
    m_DE.DE = 0x0000;
    m_HL.HL = 0x0000;
    m_SP = 0xFFFE;
    m_PC = 0x0100;

    m_cycles = 0;
}

inline uint8_t Z80Processor::ReadByte(uint16_t addr)
{
    return m_bus->ReadByte(addr);
}

inline void Z80Processor::WriteByte(uint16_t addr, uint8_t data)
{
    m_bus->WriteByte(addr, data);
}

bool Z80Processor::GetByteFromRegisterIndex(uint8_t index, uint8_t& data)
{
    data = 0;
    bool extraCycle = false;

    switch(index)
    {
    case 0:
        data = m_BC.B;
        break;
    case 1:
        data = m_BC.C;
        break;
    case 2:
        data = m_DE.D;
        break;
    case 3:
        data = m_DE.E;
        break;
    case 4:
        data = m_HL.L;
        break;
    case 5:
        data = m_HL.H;
        break;
    case 6:
        data = ReadByte(m_HL.HL);
        extraCycle = true;
        break;
    case 7:
        data = m_AF.A;
        break;
    default:
        data = 0;
        break;
    }

    return extraCycle;
}

// Dispatchers
uint8_t Z80Processor::DecodeOpcodeAndCall(uint8_t opcode)
{
    auto opCall = m_opcodesMap[opcode];
    return (this->*opCall)(opcode);
}

/////////////////////////////////////////////////////////////////////
// Opcodes implementation
/////////////////////////////////////////////////////////////////////

// Load operations
uint8_t Z80Processor::LD(uint8_t opcode)
{
    return 0;
}   

uint8_t Z80Processor::LDH(uint8_t opcode)
{
    return 0;
}

// Arithmetic/Logic instructions
uint8_t Z80Processor::ADD(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::ADC(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SUB(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SBC(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::AND(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::OR(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::XOR(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::CP(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::DEC(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::INC(uint8_t opcode)
{
    return 0;
}

// Bit operations instructions

// BIT Op
// Compare a specific bit in a given data
// and set the zero flag accordingly
// The opcode encode the bit we want to check and the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                 => 2 cycles
// Index 6          -> read memory pointed by HL => 3 cycles
//
// Bit to check is encoded in bit 3 to 5 (0x38)
uint8_t Z80Processor::BIT(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    uint8_t bitToCheck = (opcode & 0x38) >> 3;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;
    if (GetByteFromRegisterIndex(registerIndex, data))
        ++nbCycles;

    SetZeroFlag(data & (1 << bitToCheck));

    return nbCycles;
}

uint8_t Z80Processor::RES(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SET(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SWAP(uint8_t opcode)
{
    return 0;
}

// Bit shift instructions
uint8_t Z80Processor::RL(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RLA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RLC(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RLCA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RR(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RRA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RRC(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RRCA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SLA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SRA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SRL(uint8_t opcode)
{
    return 0;
}

// Dispatcher instruction
uint8_t Z80Processor::DISP(uint8_t /*opcode*/)
{
    // First get the second opcode
    uint8_t opcode = ReadByte(m_PC++);
    
    switch (opcode >> 4)
    {
    case 0x0: // RLC and RRC
        return (opcode & 0x08) ? RRC(opcode) : RLC(opcode);
    case 0x1: // RL and RR
        return (opcode & 0x08) ? RR(opcode) : RL(opcode);
    case 0x2: // SLA and SRA
        return (opcode & 0x08) ? SRA(opcode) : SLA(opcode);
    case 0x3: // SWAP and SRL
        return (opcode & 0x08) ? SRL(opcode) : SWAP(opcode);

    case 0x4: // fall-through
    case 0x5: // fall-through
    case 0x6: // fall-through
    case 0x7: // BIT
        return BIT(opcode);

    case 0x8: // fall-through
    case 0x9: // fall-through
    case 0xA: // fall-through
    case 0xB: // RES
        return RES(opcode);

    case 0xC: // fall-through
    case 0xD: // fall-through
    case 0xE: // fall-through
    case 0xF: // SET
        return SET(opcode);
    };

    return 0;
}

// Jumps and Subroutines
uint8_t Z80Processor::CALL(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::JP(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::JR(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RET(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RETI(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::RST(uint8_t opcode)
{
    return 0;
}

// Stack operations
uint8_t Z80Processor::POP(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::PUSH(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::LDSP(uint8_t opcode)
{
    return 0;
}

// Misc instructions
uint8_t Z80Processor::CCF(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::CPL(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::DAA(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::DI(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::EI(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::HALT(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::NOP(uint8_t opcode)
{
    return 0;
}

uint8_t Z80Processor::SCF(uint8_t opcode)
{
    return 0;
}


uint8_t Z80Processor::STOP(uint8_t opcode)
{
    return 0;
}

// Invalid instruction
uint8_t Z80Processor::XXX(uint8_t opcode)
{
    return 0;
}
