#pragma once

#include <memory>

namespace GBEmulator
{
    class Bus;
    namespace Utils
    {
        class IWriteVisitor;
    }

    class InstructionLogger
    {
    public:
        InstructionLogger(const GBEmulator::Bus& bus)
            : m_bus(bus)
        {}

        void OpenFileVisitor(const std::string& filepath);

        void WriteCurrentState();
        void Reset();

    private:
        const GBEmulator::Bus& m_bus;
        std::unique_ptr<Utils::IWriteVisitor> m_visitor;
    };
}