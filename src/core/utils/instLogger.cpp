#include <core/utils/instLogger.h>
#include <core/utils/fileVisitor.h>
#include <core/utils/disassenbly.h>
#include <core/bus.h>

using GBEmulator::Bus;
using GBEmulator::Utils::FileWriteVisitor;
using GBEmulator::InstructionLogger;

void InstructionLogger::OpenFileVisitor(const std::string& filepath)
{
    if (m_visitor)
        m_visitor.release();

    m_visitor = std::make_unique<FileWriteVisitor>(filepath, false, false);

    if (!reinterpret_cast<FileWriteVisitor*>(m_visitor.get())->IsValid())
        m_visitor.reset();
}

void InstructionLogger::WriteCurrentState()
{
    if (!m_visitor)
        return;

    std::string currentInst = GBEmulator::Disassemble(m_bus, m_bus.GetCPU().GetPC(), 1)[0];
    currentInst += '\n';
    m_visitor->Write(currentInst.c_str(), currentInst.size());
}

void InstructionLogger::Reset()
{
    if (!m_visitor)
        return;

    const char str[] = "-----RESET-----\n";
    m_visitor->Write(str, sizeof(str) - 1); // - 1 to remove '\0'

    WriteCurrentState();
}