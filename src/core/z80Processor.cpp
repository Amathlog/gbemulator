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

// Oversimplification of the clock mecanism
// Each operation will be done in one shot, regardless of their
// number of cycles. Each time we clock, we only decrement the cycle
// counter if it is not zero.
void Z80Processor::Clock()
{
    // IME enabling is done the next cycle after a EI instruction.
    // Handle it here.
    if (m_IMEScheduled)
    {
        m_IMEScheduled = false;
        m_IMEEnabled = true;
    }

    if (m_cycles == 0)
    {
        uint8_t opcode = ReadByte(m_PC++);
        m_cycles = DecodeOpcodeAndCall(opcode);
    }

    m_cycles--;
}

bool Z80Processor::ReadByteFromRegisterIndex(uint8_t index, uint8_t& data)
{
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
        data = m_HL.H;
        break;
    case 5:
        data = m_HL.L;
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

bool Z80Processor::WriteByteToRegisterIndex(uint8_t index, uint8_t data)
{
    bool extraCycle = false;

    switch(index)
    {
    case 0:
        m_BC.B = data;
        break;
    case 1:
        m_BC.C = data;
        break;
    case 2:
        m_DE.D = data;
        break;
    case 3:
        m_DE.E = data;
        break;
    case 4:
        m_HL.H = data;
        break;
    case 5:
        m_HL.L = data;
        break;
    case 6:
        WriteByte(m_HL.HL, data);
        extraCycle = true;
        break;
    case 7:
        m_AF.A = data;
        break;
    default:
        break;
    }

    return extraCycle;
}

void Z80Processor::ReadWordFromRegisterIndex(uint8_t index, uint16_t& data)
{
    switch(index)
    {
    case 0:
        data = m_BC.BC;
        break;
    case 1:
        data = m_DE.DE;
        break;
    case 2:
        data = m_HL.HL;
        break;
    case 3:
        data = m_SP;
        break;
    default:
        data = 0;
        break;
    }
}

void Z80Processor::WriteWordToRegisterIndex(uint8_t index, uint16_t data)
{
    switch(index)
    {
    case 0:
        m_BC.BC = data;
        break;
    case 1:
        m_DE.DE = data;
        break;
    case 2:
        m_HL.HL = data;
        break;
    case 3:
        m_SP = data;
        break;
    default:
        break;
    }
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
// ADD op
// General addition
// There are a lot of cases:
// ADD A,r8: 0x80 -> 0x87: Add the value from a given register to A (1 or 2 cycles)
// ADD HL,r16: 0x09/0x19/0x29/0x39: Add the value from a given register to HL (2 cycles)
// ADD A,d8: 0xC6: Add the litteral value to A (2 cycles)
// ADD SP,e8: 0xE8: Add the litteral signed value to SP (4 cycles)
uint8_t Z80Processor::ADD(uint8_t opcode)
{
    // Signed addition
    if (opcode == 0xE8)
    {
        int8_t data = (int8_t)(ReadByte(m_PC++));
        m_AF.F.H = ((data > 0) && ((m_SP & 0x000F) == 0x0F));
        m_AF.F.C = ((data > 0) && ((m_SP & 0x0080) > 0));

        m_SP += data;
        m_AF.F.Z = 0;
        m_AF.F.N = 0;
        return 4;
    }

    // 16 bits mode
    if ((opcode & 0x0F) == 0x09)
    {
        uint8_t index = opcode >> 4;
        uint16_t data = 0;
        ReadWordFromRegisterIndex(index, data);

        m_AF.F.H = (data > 0) && (m_HL.HL & 0x0FFF) == 0x0FFF;

        uint32_t temp = (uint32_t)data + (uint32_t)m_HL.HL;

        m_AF.F.C = (temp & 0xFFFF0000) > 0;
        m_HL.HL = (uint16_t)(temp & 0x0000FFFF);
        m_AF.F.N = 0;

        return 2;
    }

    // In the other cases, it's exactly the same than ADC
    // but with a carry equal to 0
    // We re-use the same code then
    uint8_t overridenOpcode = opcode == 0xC6 ? 0xCE : opcode;
    m_AF.F.C = 0;
    return ADC(overridenOpcode);
}

// ADC op
// Add the given data to A, with the carry
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 0
// H: If overflow from bit 3
// C: If overflow from bit 7
uint8_t Z80Processor::ADC(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xCE)
    {
        // Literal value
        data = ReadByte(m_PC++);
        nbCycles++;
    }
    else
    {
        uint8_t index = opcode & 0x07;
        if (ReadByteFromRegisterIndex(index, data))
            nbCycles++;
    }

    m_AF.F.H = (data > 0 || m_AF.F.C) && (m_AF.A & 0x0F) == 0x0F;

    uint16_t temp = (uint16_t)m_AF.A + (uint16_t)data + (uint16_t)m_AF.F.C;

    m_AF.F.N = 0;
    m_AF.F.C = (temp & 0xFF00) > 0;

    m_AF.A = (uint8_t)(temp & 0x00FF);

    SetZeroFlag(m_AF.A);

    return nbCycles;
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

// DEC op
// Decrement the value of the given data
// and write it back
// Lowest 4 bits:
// - 0xB     -> 16 bits mode
// - 0x5/0xD -> 8 bits mode
// 2 cycles for 16 bits mode
// 1 cycle for 8 bit mode in registers
// 3 cycles for 8 bit mode in memory
//
// Flags (untouched in 16bits mode):
// Z: If result is zero
// N: 1
// H: Set if borrow from bit 4.
// C: Untouched
uint8_t Z80Processor::DEC(uint8_t opcode)
{
    if ((opcode & 0x0F) == 0x0B)
    {
        // 16 bits mode
        uint16_t data = 0;
        uint8_t index = opcode >> 4;
        ReadWordFromRegisterIndex(index, data);

        data--;
        WriteWordToRegisterIndex(index, data);

        return 2;
    }

    // 8 bit mode
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    uint8_t lowestBitIndex = (opcode & 0x08) > 0;
    uint8_t index = ((opcode >> 4) << 1) | lowestBitIndex;

    if(ReadByteFromRegisterIndex(index, data))
        nbCycles++;

    // TODO: Check this one
    // In some docs, it's written: Set if borrow from bit 4.
    // Not exactly sure if that's the right thing here.
    m_AF.F.H = ((data & 0xF0) > 0) & ((data & 0x0F) == 0);

    data--;

    if (WriteByteToRegisterIndex(index, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 1;

    return nbCycles;
}

// INC op
// Increment the value of the given data
// and write it back
// Lowest 4 bits:
// - 0x3     -> 16 bits mode
// - 0x4/0xC -> 8 bits mode
// 2 cycles for 16 bits mode
// 1 cycle for 8 bit mode in registers
// 3 cycles for 8 bit mode in memory
//
// Flags (untouched in 16bits mode):
// Z: If result is zero
// N: 0
// H: Set if overflow from bit 3
// C: Untouched
uint8_t Z80Processor::INC(uint8_t opcode)
{
    if ((opcode & 0x0F) == 0x03)
    {
        // 16 bits mode
        uint16_t data = 0;
        uint8_t index = opcode >> 4;
        ReadWordFromRegisterIndex(index, data);

        data++;
        WriteWordToRegisterIndex(index, data);

        return 2;
    }

    // 8 bit mode
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    uint8_t lowestBitIndex = (opcode & 0x08) > 0;
    uint8_t index = ((opcode >> 4) << 1) | lowestBitIndex;

    if(ReadByteFromRegisterIndex(index, data))
        nbCycles++;

    m_AF.F.H = (data & 0x0F) == 0x0F;

    data++;
    if (WriteByteToRegisterIndex(index, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;

    return nbCycles;
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
//
// Flags:
// Z: If the given bit is 0
// N: 0
// H: 1
// C: Untouched
uint8_t Z80Processor::BIT(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    uint8_t bitToCheck = (opcode & 0x38) >> 3;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;
    if (ReadByteFromRegisterIndex(registerIndex, data))
        ++nbCycles;

    SetZeroFlag(data & (1 << bitToCheck));
    m_AF.F.N = 0;
    m_AF.F.H = 1;

    return nbCycles;
}

// RES Op
// Set a specific bit in a given data to 0
// The opcode encode the bit we want to reset and the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Bit to set is encoded in bit 3 to 5 (0x38)
//
// Flags: Untouched
uint8_t Z80Processor::RES(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    uint8_t bitToCheck = (opcode & 0x38) >> 3;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    data = data & (~(1 << bitToCheck));
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    return nbCycles;
}

// SET Op
// Set a specific bit in a given data to 1
// The opcode encode the bit we want to reset and the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Bit to set is encoded in bit 3 to 5 (0x38)
//
// Flags: Untouched
uint8_t Z80Processor::SET(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    uint8_t bitToCheck = (opcode & 0x38) >> 3;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    data = data | (1 << bitToCheck);
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    return nbCycles;
}

// SWAP Op
// Swap 4 most significant bits with the 4 less significant bits
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: 0
uint8_t Z80Processor::SWAP(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    data = (data << 4) | (data >> 4);
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;
    m_AF.F.C = 0;

    return nbCycles;
}

// Bit shift instructions

// RL Op
// Shift a given data to the left and introduce the carry to the right
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::RL(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    uint8_t oldCarry = m_AF.F.C;
    m_AF.F.C = (data & 0x80) > 0;

    data = (data << 1) | oldCarry;
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// RLA op
// Same as RL op, but special op for the A register
// done in 1 cycle instead of 2
// Register A is index 7
uint8_t Z80Processor::RLA(uint8_t opcode)
{
    RL(0x07);
    return 1;
}

// RLC Op
// Shift a given data to the left and reintroduce the highest bit at the right.
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::RLC(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    m_AF.F.C = (data & 0x80) > 0;

    data = (data << 1) | m_AF.F.C;
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// RLCA op
// Same as RLC op, but special op for the A register
// done in 1 cycle instead of 2
// Register A is index 7
uint8_t Z80Processor::RLCA(uint8_t opcode)
{
    RLC(0x07);
    return 1;
}

// RR Op
// Shift a given data to the right and introduce the carry to the left
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::RR(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    uint8_t oldCarry = m_AF.F.C;
    m_AF.F.C = data & 0x01;

    data = (data >> 1) | (oldCarry << 7);
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// RRA op
// Same as RR op, but special op for the A register
// done in 1 cycle instead of 2
// Register A is index 7
uint8_t Z80Processor::RRA(uint8_t /*opcode*/)
{
    RR(0x07);
    return 1;
}

// RRC Op
// Shift a given data to the right and reintroduce the lowest bit to the left
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::RRC(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    m_AF.F.C = data & 0x01;

    data = (data >> 1) | (m_AF.F.C << 7);
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// RRCA op
// Same as RRC op, but special op for the A register
// done in 1 cycle instead of 2
// Register A is index 7
uint8_t Z80Processor::RRCA(uint8_t /*opcode*/)
{
    RRC(0x07);
    return 1;
}

// SLA Op: Left arithmetic shift
// Shift a given data to the left
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::SLA(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    m_AF.F.C = (data & 0x80) > 0;

    data <<= 1;
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// SRA Op : Right arithmetic shift
// Shift a given data to the right. Most significant bit stays unchanged.
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::SRA(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    m_AF.F.C = (data & 0x01) > 0;

    data = (data >> 1) | (data & 0x80);
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
}

// SRA Op : Right logical shift
// Shift a given data to the right.
// The opcode encode the register index
// 
// Index is encoded in the 3 lowest bits (0x07)
// Index [0, 5] + 7 -> registers                                => 2 cycles
// Index 6          -> read memory pointed by HL and write back => 4 cycles
//
// Flags:
// Z: If result is 0
// N: 0
// H: 0
// C: If we have an overflow
uint8_t Z80Processor::SRL(uint8_t opcode)
{
    uint8_t nbCycles = 2;
    
    uint8_t registerIndex = opcode & 0x07;
    uint8_t data = 0;

    if (ReadByteFromRegisterIndex(registerIndex, data))
        nbCycles++;

    m_AF.F.C = (data & 0x01) > 0;

    data >>= 1;
    
    if (WriteByteToRegisterIndex(registerIndex, data))
        nbCycles++;

    SetZeroFlag(data);
    m_AF.F.N = 0;
    m_AF.F.H = 0;

    return nbCycles;
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

// POP op
// Pop a word from the stack to the given register
// Index is encoded in the opcode
// Nb cycles: 3
//
// Flags:
// Untouched if register BC, DE or HL
// Set accordingly if register AF
uint8_t Z80Processor::POP(uint8_t opcode)
{
    uint16_t data = 0;
    uint8_t index = (opcode >> 4) & 0x03;
    
    uint8_t lowData = ReadByte(m_SP++);
    uint8_t highData = ReadByte(m_SP++);

    data = ((uint16_t)highData << 8) | lowData;

    WriteWordToRegisterIndex(index, data);

    return 3;
}

// PUSH op
// Push a word to the stack from the given register
// Index is encoded in the opcode
// Nb cycles: 4
//
// Flags: Untouched
uint8_t Z80Processor::PUSH(uint8_t opcode)
{
    uint16_t data = 0;
    uint8_t index = (opcode >> 4) & 0x03;

    ReadWordFromRegisterIndex(index, data);

    WriteByte(--m_SP, (uint8_t)(data & 0x00FF));
    WriteByte(--m_SP, (uint8_t)(data >> 8));

    return 4;
}

uint8_t Z80Processor::LDSP(uint8_t opcode)
{
    return 0;
}

// Misc instructions

// CCF op
// Complement carry flag: C = ~C
// Nb cycles: 1
//
// Flags:
// N: 0
// H: 0
// C: Inverted
uint8_t Z80Processor::CCF(uint8_t /*opcode*/)
{
    m_AF.F.C = ~m_AF.F.C;
    return 1;
}

// CPL op
// Complement accumulator: A = ~A
// Nb cycles: 1
//
// Flags:
// N: 1
// H: 1
// Z and C: Untouched
uint8_t Z80Processor::CPL(uint8_t /*opcode*/)
{
    m_AF.A = ~m_AF.A;
    m_AF.F.N = 1;
    m_AF.F.H = 1;
    return 1;
}

// DAA op
// Adjust the result of a binary addition/subtraction to
// retroactively turn it into a BCD addition/subtraction
// cf https://en.wikipedia.org/wiki/Binary-coded_decimal
// It uses flags N and H and adjust the value in the accumulator (A)
// Takes 1 cycle
//
// Flags:
// Z: If result is 0
// H: 0
// N: Untouched
// C: Set or reset, depending on the operation
// Code was taken from https://forums.nesdev.org/viewtopic.php?t=15944
// need to do another pass to understand it better
uint8_t Z80Processor::DAA(uint8_t /*opcode*/)
{
    // Addition, adjust if (half-)carry occured or 
    // is the result is out-of-bounds
    if (m_AF.F.N == 0)
    {
        if (m_AF.F.C == 1 || (m_AF.A > 0x99))
        {
            m_AF.A += 0x60;
            m_AF.F.C = 1;
        }
        if (m_AF.F.H == 1 || (m_AF.A & 0x0f) > 0x09)
        {
            m_AF.A += 0x06;
        }
    }
    // Subtraction, only adjust if (half-)carry occured
    else
    {
        if (m_AF.F.C)
        {
            m_AF.A -= 0x60;
        }
        if (m_AF.F.H)
        {
            m_AF.A -= 0x06;
        }
    }

    SetZeroFlag(m_AF.A);
    m_AF.F.H = 0;
    
    return 1;
}

// DI op
// Disable the IME flag. Immediate
// Nb cycles: 1
//
// Flags: Untouched
uint8_t Z80Processor::DI(uint8_t /*opcode*/)
{
    m_IMEEnabled = false;
    return 1;
}

// EI op
// Enable the IME flag at the next cycle (cf comment in Clock function)
// Nb cycles: 1
//
// Flags: Untouched
uint8_t Z80Processor::EI(uint8_t /*opcode*/)
{
    m_IMEScheduled = true;
    return 1;
}

uint8_t Z80Processor::HALT(uint8_t opcode)
{
    return 0;
}

// NOP op
// Do nothing
// Nb cycles: 1
//
// Flags: Untouched
uint8_t Z80Processor::NOP(uint8_t /*opcode*/)
{
    return 1;
}

// SCF op
// Set carry flag to 1
// 1 cycle
// 
// Flags:
// C: 1
// N: 0
// H: 0
uint8_t Z80Processor::SCF(uint8_t /*opcode*/)
{
    m_AF.F.C = 1;
    m_AF.F.N = 0;
    m_AF.F.H = 0;
    return 1;
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
