#include <core/z80Processor.h>

using GBEmulator::Z80Processor;

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
    m_SP = 0x0000;
    m_PC = 0x0000;

    m_cycles = 0;
}