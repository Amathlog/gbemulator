#include "core/utils/debugger.h"
#include <any>
#include <fstream>
#include <iostream>
#include <sstream>

namespace GBEmulator
{
namespace Utils
{

Debugger& Debugger::GetInstance()
{
    static Debugger s_instance;
    return s_instance;
}

void Debugger::LoadSymbols(const std::filesystem::path& path)
{
    m_symbols.clear();

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to open symbols file: " << path << std::endl;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        uint16_t address, romBank;
        std::string symbol;
        char colon;
        if (!(iss >> std::hex >> romBank >> colon >> address >> symbol) || colon != ':')
        {
            continue; // Skip invalid lines
        }

        m_symbols[address][romBank] = std::move(symbol);
    }
}

void Debugger::Reset()
{
    while (!m_callStack.empty())
    {
        m_callStack.pop();
    }
}

void Debugger::PushExecutionState(const ExecutionState& state)
{
    m_executionBuffer.Push(state);

    switch (state.status)
    {
    case GBEmulator::Utils::ExecutionStatus::FunctionCall:
    case GBEmulator::Utils::ExecutionStatus::Interruption:
        CallHappened(state);
        break;
    case GBEmulator::Utils::ExecutionStatus::FunctionReturn:
        ReturnHappened();
        break;
    default:
        break;
    }
}

void Debugger::CallHappened(const ExecutionState& state)
{
    m_callStack.push(state);

    const uint16_t pc = state.PC;
    const uint16_t romBank = pc < 0x4000 ? 0 : state.romBank;

    // Find symbol
    if (!m_symbols.empty())
    {
        auto it = m_symbols.find(pc);
        if (it != m_symbols.end())
        {
            auto it2 = it->second.find(romBank);
            if (it2 != it->second.end())
            {
                // std::cout << "Call at " << std::hex << romBank << ":" << state.PC << ": " << it2->second <<
                // std::endl;
            }
        }
        else
        {
            // Interupt addresses might not have symbols
            static constexpr std::array<uint16_t, 5> interupt_addresses = {0x40, 0x48, 0x50, 0x58, 0x60};
            for (uint16_t interupt_address : interupt_addresses)
            {
                if (pc == interupt_address)
                {
                    return;
                }
            }

            std::cout << "Invalid symbol" << std::endl;
        }
    }
}

void Debugger::ReturnHappened()
{
    if (!m_callStack.empty())
    {
        m_callStack.pop();
    }
    else
    {
        std::cout << "Callstack negative!" << std::endl;
    }
}

} // namespace Utils
} // namespace GBEmulator
