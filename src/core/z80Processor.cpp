#include <core/z80Processor.h>
#include <core/bus.h>
#include <cstdint>
#include <array>
#include <sys/types.h>

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
// Will return true if the next clock will execute a new instruction
bool Z80Processor::Clock()
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
        uint8_t opcode = FetchByte();
        m_cycles = DecodeOpcodeAndCall(opcode);
    }

    m_cycles--;

    return m_cycles == 0;
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

inline void Z80Processor::PushWordToStack(uint16_t data)
{
    // MSB
    WriteByte(--m_SP, (uint8_t)(data >> 8));
    // LSB
    WriteByte(--m_SP, (uint8_t)(data & 0x00FF));
}

inline uint16_t Z80Processor::PopWordFromStack()
{
    uint16_t lowData = ReadByte(m_SP++);
    uint16_t highData = ReadByte(m_SP++);

    return (highData << 8) | lowData;
}

inline uint8_t Z80Processor::FetchByte()
{
    return ReadByte(m_PC++);
}

inline uint16_t Z80Processor::FetchWord()
{
    uint16_t lsb = (uint16_t)FetchByte();
    uint16_t msb = (uint16_t)FetchByte();
    return (msb << 8) | lsb;
}

constexpr const char* Z80Processor::GetDebugStringForOp(uint8_t opcode)
{
    auto opFunc = m_opcodesMap[opcode];

#define TEST_FUNC(x) if (opFunc == &Z80Processor::x) return #x

    TEST_FUNC(LD);   TEST_FUNC(LDH);

    // Arithmetic/Logic instructions
    TEST_FUNC(ADD);  TEST_FUNC(ADC);  TEST_FUNC(SUB);
    TEST_FUNC(SBC);  TEST_FUNC(AND);  TEST_FUNC(OR);
    TEST_FUNC(XOR);  TEST_FUNC(CP);   TEST_FUNC(DEC);
    TEST_FUNC(INC);

    // Bit operations instructions
    TEST_FUNC(BIT);  TEST_FUNC(RES);  TEST_FUNC(SET);
    TEST_FUNC(SWAP);

    // Bit shift instructions
    TEST_FUNC(RL);   TEST_FUNC(RLA);  TEST_FUNC(RLC);
    TEST_FUNC(RLCA); TEST_FUNC(RR);   TEST_FUNC(RRA);
    TEST_FUNC(RRC);  TEST_FUNC(RRCA); TEST_FUNC(SLA);
    TEST_FUNC(SRA);  TEST_FUNC(SRL);

    // Dispatcher instruction
    TEST_FUNC(DISP);

    // Jumps and Subroutines
    TEST_FUNC(CALL); TEST_FUNC(JP);   TEST_FUNC(JR);
    TEST_FUNC(RET);  TEST_FUNC(RETI); TEST_FUNC(RST);

    // Stack operations
    TEST_FUNC(POP);  TEST_FUNC(PUSH); TEST_FUNC(LDSP);

    // Misc instructions
    TEST_FUNC(CCF);  TEST_FUNC(CPL);  TEST_FUNC(DAA);
    TEST_FUNC(DI);   TEST_FUNC(EI);   TEST_FUNC(HALT);
    TEST_FUNC(NOP);  TEST_FUNC(SCF);  TEST_FUNC(STOP);

#undef TEST_FUNC

    return "XXX";
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

// LD op
// Transfer data from one place to the other. It can be registers (8 or 16 bits),
// litteral values (8 or 16 bits) or memory.
// The rule for the number of cycles is:
// 8 bits register to 8 bits registers: 1 cycle
// Read from memory: +1 cycle
// Write to memory: +1 cycle
// Read a litteral byte: +1 cycle
// Read a litteral word: +2 cycles
// Flags: Untouched for all except 7th case
uint8_t Z80Processor::LD(uint8_t opcode)
{
    // First 4 rows of instructions
    if (opcode < 0x40)
    {
        uint8_t filteredOpcode = (opcode & 0x0F);
        uint8_t index = (opcode >> 4) & 0x03;

        // 1st case: Litteral word to 16bits register
        // 3 cycles
        if (filteredOpcode == 0x01)
        {
            uint16_t data = FetchWord();
            WriteWordToRegisterIndex(index, data);
            return 3;
        }

        // 2nd case: Accumulator to memory
        // 3rd case: Accumulator from memory
        // Index 2 and 3 are a bit different, since they designed both HL
        // but it will be incremented (index 2) or decremented (index 3)
        // after read/write
        // 2 cycles
        if (filteredOpcode == 0x02 || filteredOpcode == 0x0A)
        {
            uint16_t& reg = index == 0 ? m_BC.BC : (index == 1 ? m_DE.DE : m_HL.HL);

            if (filteredOpcode == 0x02)
                WriteByte(reg, m_AF.A);
            else
                m_AF.A = ReadByte(reg);

            if (index == 2)
                reg++;
            else if (index == 3)
                reg--;

            return 2;
        }

        // 4th case: Stack pointer to memory (pointed by litteral word a16)
        // Store LSB to a16 and MSB to a16 + 1
        // 5 cycles
        if (filteredOpcode == 0x08)
        {
            uint16_t addr = FetchWord();
            WriteByte(addr, (uint8_t)(m_SP & 0x00FF));
            WriteByte(addr + 1, (uint8_t)(m_SP >> 8));
            return 5;
        }

        // 5th case: Litteral to register/memory
        // 2 or 3 cycles
        index = (index << 1) | ((filteredOpcode & 0x08) > 0);
        uint8_t data = FetchByte();
        return WriteByteToRegisterIndex(index, data) ? 3 : 2;
    }

    // 6th case: Accumulator to/from memory pointed by $FF00 + Carry
    // 2 cycles
    if (opcode == 0xE2 || opcode == 0xF2)
    {
        uint16_t addr = 0xFF00 + (m_AF.F.C);
        if (opcode == 0xE2)
            WriteByte(addr, m_AF.A);
        else
            m_AF.A = ReadByte(addr);

        return 2;
    }

    // 7th case: Load HL in SP
    // 2 cycles
    if (opcode == 0xF9)
    {
        m_SP = m_HL.HL;
        return 2;
    }

    // 8th case: Accumulator to/from memory pointed by litteral word
    // 4 cycles
    if (opcode == 0xEA || opcode == 0xFA)
    {
        uint16_t addr = FetchWord();
        if (opcode == 0xEA)
            WriteByte(addr, m_AF.A);
        else
            m_AF.A = ReadByte(addr);

        return 4;
    }

    // Final case: Register to register (or memory pointed by HL)
    uint8_t writeIndex = (((opcode & 0x70) >> 5) << 1) | ((opcode & 0x08) > 0);
    uint8_t readIndex = (opcode & 0x07);
    uint8_t nbCycles = 1;
    uint8_t data = 0;
    if (ReadByteFromRegisterIndex(readIndex, data))
        nbCycles++;
    if (WriteByteToRegisterIndex(writeIndex, data))
        nbCycles++;

    return nbCycles;
}   

// LDH op
// Transfer accumulator to/from memory. Memory address is an offset of 1 byte
// Offseting from 0xFF00
// Nb cycles: 2
// Flags: Untouched
uint8_t Z80Processor::LDH(uint8_t opcode)
{
    uint8_t offset = FetchByte();
    uint16_t addr = 0xFF00 | offset;
    if (opcode == 0xE0)
        m_AF.A = ReadByte(addr);
    else
        WriteByte(addr, m_AF.A);

    return 2;
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
        int8_t data = (int8_t)(FetchByte());
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
        data = FetchByte();
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

// SUB op
// General subtraction
// There are a less cases than addition:
// SUB A,r8: 0x80 -> 0x87: Subtract the value from a given register to A (1 or 2 cycles)
// SUB A,d8: 0xC6: Subtract the litteral value to A (2 cycles)
uint8_t Z80Processor::SUB(uint8_t opcode)
{
    // In all cases, it's exactly the same than SBC
    // but with a carry equal to 0
    // We re-use the same code then
    uint8_t overridenOpcode = opcode == 0xD6 ? 0xDE : opcode;
    m_AF.F.C = 0;
    return SBC(overridenOpcode);
}

// SBC op
// Subtract the given data to A, with the carry
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 1
// H: If borrow from bit 4
// C: If borrow (if data + carry > A)
uint8_t Z80Processor::SBC(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xDE)
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

    m_AF.F.H = (data > 0 || m_AF.F.C) && (m_AF.A & 0x0F) == 0x00;
    uint8_t oldCarry = m_AF.F.C;
    m_AF.F.C = (data + m_AF.F.C) > m_AF.A;

    m_AF.A -= (data + oldCarry);

    m_AF.F.N = 1;

    SetZeroFlag(m_AF.A);

    return nbCycles;
}

// AND op
// Bitwise and between the given data and A
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 0
// H: 1
// C: 0
uint8_t Z80Processor::AND(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xE6)
    {
        // Literal value
        data = FetchByte();
        nbCycles++;
    }
    else
    {
        uint8_t index = opcode & 0x07;
        if (ReadByteFromRegisterIndex(index, data))
            nbCycles++;
    }

    m_AF.F.H = 1;
    m_AF.F.C = 0;
    m_AF.F.N = 0;

    m_AF.A &= data;

    SetZeroFlag(m_AF.A);

    return nbCycles;
}

// OR op
// Bitwise or between the given data and A
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 0
// H: 0
// C: 0
uint8_t Z80Processor::OR(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xF6)
    {
        // Literal value
        data = FetchByte();
        nbCycles++;
    }
    else
    {
        uint8_t index = opcode & 0x07;
        if (ReadByteFromRegisterIndex(index, data))
            nbCycles++;
    }

    m_AF.F.H = 0;
    m_AF.F.C = 0;
    m_AF.F.N = 0;

    m_AF.A |= data;
    
    SetZeroFlag(m_AF.A);

    return nbCycles;
}

// XOR op
// Bitwise xor between the given data and A
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 0
// H: 0
// C: 0
uint8_t Z80Processor::XOR(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xEE)
    {
        // Literal value
        data = FetchByte();
        nbCycles++;
    }
    else
    {
        uint8_t index = opcode & 0x07;
        if (ReadByteFromRegisterIndex(index, data))
            nbCycles++;
    }

    m_AF.F.H = 0;
    m_AF.F.C = 0;
    m_AF.F.N = 0;

    m_AF.A ^= data;
    
    SetZeroFlag(m_AF.A);

    return nbCycles;
}

// CP op
// Like general subtraction, but without storing the result.
// Used to compare values, and set flags
// Nb Cycles:
// From register: 1 cycle
// From memory or literal value: 2 cycles
//
// Flags:
// Z: If the result is 0
// N: 1
// H: If borrow from bit 4
// C: If borrow (data > A)
uint8_t Z80Processor::CP(uint8_t opcode)
{
    uint8_t data = 0;
    uint8_t nbCycles = 1;
    if (opcode == 0xFE)
    {
        // Literal value
        data = FetchByte();
        nbCycles++;
    }
    else
    {
        uint8_t index = opcode & 0x07;
        if (ReadByteFromRegisterIndex(index, data))
            nbCycles++;
    }

    m_AF.F.H = (data > 0) && (m_AF.A & 0x0F) == 0x00;
    m_AF.F.C = data > m_AF.A;
    m_AF.F.N = 1;
    m_AF.F.Z = m_AF.A == data;

    return nbCycles;
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
    uint8_t opcode = FetchByte();
    
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
// CALL op
// Call address given by litteral word.
// Will push the address of the next instruction on the stack
// and then set the PC to the wanted address
// Nb cycles: 6
// Can also be a conditional call (depending on flags).
// If the condition is not met it takes 3 cycles (6 otherwise)
uint8_t Z80Processor::CALL(uint8_t opcode)
{
    // Always read the address, even if we don't jump
    uint16_t addr = FetchWord();

    // 0xC4 => Z not set
    // 0xD4 => C not set
    // 0xCC => Z set
    // 0xDC => C set
    // 0xCD => Always jump
    bool conditionMet = 
        (opcode == 0xC4 && !m_AF.F.Z) ||
        (opcode == 0xD4 && !m_AF.F.C) ||
        (opcode == 0xCC && m_AF.F.Z) ||
        (opcode == 0xDC && m_AF.F.C) ||
        (opcode == 0xCD);

    if (!conditionMet)
        return 3;

    // Next instrcution is push on the stack
    PushWordToStack(m_PC);

    // Then jump
    m_PC = addr;

    return 6;
}

// JP op
// Jump to address given by litteral word.
// Set the PC to the wanted address
// JP n16: Jump to address n16, nb cycles 4
// JP cc,n16: Conditional jump, depending on flags. Nb cycles 4 (3 if condition not met)
// JP HL: Jump to address pointed by HL, nb cycles 1
uint8_t Z80Processor::JP(uint8_t opcode)
{
    // Jump to HL
    if (opcode == 0xE9)
    {
        m_PC = m_HL.HL;
        return 1;
    }

    // Always read the address, even if we don't jump
    uint16_t addr = FetchWord();

    // 0xC2 => Z not set
    // 0xD2 => C not set
    // 0xCA => Z set
    // 0xDA => C set
    // 0xC3 => Always jump
    bool conditionMet = 
        (opcode == 0xC2 && !m_AF.F.Z) ||
        (opcode == 0xD2 && !m_AF.F.C) ||
        (opcode == 0xCA && m_AF.F.Z) ||
        (opcode == 0xDA && m_AF.F.C) ||
        (opcode == 0xC3);

    if (!conditionMet)
        return 3;

    m_PC = addr;

    return 4;
}

// JR op
// Relative jump. Add the literral signed value to the current PC
// Can be conditional, depending on the flags.
// Nb cycles: 3 if jump, 2 if no jump
// Flags: Untouched
uint8_t Z80Processor::JR(uint8_t opcode)
{
    // Always read the litteral, even if we don't jump
    int8_t offset = (int8_t)FetchByte();

    // 0x20 => Z not set
    // 0x30 => C not set
    // 0x28 => Z set
    // 0x38 => C set
    // 0x18 => Always jump
    bool conditionMet = 
        (opcode == 0x20 && !m_AF.F.Z) ||
        (opcode == 0x30 && !m_AF.F.C) ||
        (opcode == 0x28 && m_AF.F.Z) ||
        (opcode == 0x38 && m_AF.F.C) ||
        (opcode == 0x18);

    if (!conditionMet)
        return 2;

    m_PC += offset;

    return 3;
}

// RET op
// Return from subroutine. Will pop the address from the stack.
// Can be conditional, depending on the flags
// Read from the stack only (and moving SP) only if we jump
// Nb cycles: 4 if no condition, 2 if no jump and 5 if jump with condition
// Flags: Untouched
uint8_t Z80Processor::RET(uint8_t opcode)
{
    // 0xC0 => Z not set
    // 0xD0 => C not set
    // 0xC8 => Z set
    // 0xD8 => C set
    // 0xC9 => Always jump
    bool conditionMet = 
        (opcode == 0xC0 && !m_AF.F.Z) ||
        (opcode == 0xD0 && !m_AF.F.C) ||
        (opcode == 0xC8 && m_AF.F.Z) ||
        (opcode == 0xD8 && m_AF.F.C) ||
        (opcode == 0xC9);

    if (!conditionMet)
        return 2;

    m_PC = PopWordFromStack();

    return (opcode == 0xC9) ? 4 : 5;
}

// RETI op
// Equivalent of EI + RET, but the IME is set immediately
// (instead of after next instruction)
// Nb cycles: 4
// Flags: Untouched
uint8_t Z80Processor::RETI(uint8_t opcode)
{
    m_PC = PopWordFromStack();
    m_IMEEnabled = true;
    return 4;
}

// RST Op
// Same than CALL but jumping to predefined locations
// (encoded in the opcode) and thus faster.
// Nb cycles: 4
// Flags: Untouched
uint8_t Z80Processor::RST(uint8_t opcode)
{
    uint8_t index = (((opcode >> 4) & 0x03) << 1) | ((opcode & 0x08) > 0);
    constexpr std::array<uint16_t, 8> addrTable = {
        0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038
    };

    m_PC = addrTable[index];

    return 4;
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
    uint16_t data = PopWordFromStack();
    uint8_t index = (opcode >> 4) & 0x03;

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

    PushWordToStack(data);

    return 4;
}

// LDSP op
// Add signed litteral to SP and store value in HL
// Nb cycles: 3
// Flags: 
// N: 0
// Z: 0
// C: If overflow bit 7
// H: If overflow bit 3
uint8_t Z80Processor::LDSP(uint8_t opcode)
{
    int8_t offset = (int8_t)FetchByte();
    m_AF.F.N = 0;
    m_AF.F.Z = 0;
    uint16_t newSP = m_SP + offset;
    m_AF.F.C = (newSP & 0xFF00) > (m_SP & 0xFF00);
    m_AF.F.H = (newSP & 0xFFF0) > (m_SP & 0xFFF0);
    m_HL.HL = newSP;
    return 3;
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
    // TODO
    return 1;
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
    // Always read next byte
    FetchByte();

    // TODO
    return 2;
}

// Invalid instruction
uint8_t Z80Processor::XXX(uint8_t opcode)
{
    return 1;
}
