#pragma once

#include "core/utils/ringBuffer.h"
#include <cstdint>
#include <filesystem>
#include <stack>
#include <string>
#include <unordered_map>

#define EXECUTION_BUFFER_SIZE 10000000

namespace GBEmulator
{
namespace Utils
{

enum class ExecutionStatus : uint8_t
{
    FunctionCall,
    FunctionReturn,
    Interruption,
    Normal
};

struct ExecutionState
{
    uint16_t PC = 0;
    uint16_t SP = 0;
    uint16_t romBank = 0;
    uint16_t ramBank = 0;

    uint16_t AFRegister = 0;
    uint16_t BCRegister = 0;
    uint16_t DERegister = 0;
    uint16_t HLRegister = 0;

    uint64_t cycles = 0;
    ExecutionStatus status = ExecutionStatus::Normal;
};

class Debugger
{
public:
    static Debugger& GetInstance();

    // Format of the symbols file: {romBank}:{address} {symbol}
    void LoadSymbols(const std::filesystem::path& path);
    void Reset();

    void CallHappened(const ExecutionState& state);
    void ReturnHappened();

    void PushExecutionState(const ExecutionState& state);
    const auto& GetSymbols() const { return m_symbols; }

protected:
    Debugger() = default;

    std::unordered_map<uint16_t /*address*/, std::unordered_map<uint16_t /*romBank*/, std::string>> m_symbols;
    std::stack<ExecutionState> m_callStack;
    RingBuffer<ExecutionState, EXECUTION_BUFFER_SIZE> m_executionBuffer;
};
} // namespace Utils
} // namespace GBEmulator