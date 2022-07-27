#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <core/bus.h>
#include <exe/utils.h>
#include <gtest/gtest.h>

namespace GBEmulatorTests
{
    inline bool RunTo(GBEmulator::Bus& bus, uint16_t address)
    {
        // Put an hard limit to avoid infinite loop
        constexpr size_t MAX_CYCLES = 1000000;
        size_t count = 0;
        bus.SetRunToAddress(address);
        while(!bus.IsInBreak() && count++ < MAX_CYCLES)
            bus.Clock();

        bus.BreakContinue();

        return count != MAX_CYCLES;
    }

    inline void RunForNCycles(GBEmulator::Bus& bus, unsigned nbCycles)
    {
        for (unsigned i = 0; i < nbCycles; ++i)
            bus.Clock();
    }

    template <typename Container>
    inline bool MemoryMatch(GBEmulator::Bus& bus, uint16_t startAddress, uint16_t endAddress, const Container& data)
    {
        static_assert(std::is_same_v<typename Container::value_type, uint8_t>);

        if (endAddress < startAddress)
            return false;

        if (endAddress - startAddress + 1 != data.size())
            return false;

        uint16_t it = startAddress;

        for (auto i = 0; i < data.size(); ++i)
        {
            if (data[i] != bus.ReadByte(it++))
                return false;
        }

        return true;
    }

    inline std::string FindTestRom(const std::string& romName)
    {
        std::filesystem::path rootPath = GBEmulatorExe::GetRootPath();

        rootPath /= "tests";

        std::stack<std::filesystem::path> listDir;
        listDir.push(std::move(rootPath));

        while (!listDir.empty())
        {
            std::filesystem::path curr = listDir.top();
            listDir.pop();

            for (auto it : std::filesystem::directory_iterator(curr))
            {
                std::filesystem::path item = it.path();

                if (it.is_directory())
                {
                    listDir.push(std::move(item));
                    continue;
                }

                if (it.is_regular_file() && item.filename().string() == romName)
                {
                    return item.string();
                }
            }
        }

        return "";
    }
}