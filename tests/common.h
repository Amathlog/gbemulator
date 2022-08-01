#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <core/bus.h>
#include <core/utils/utils.h>
#include <gtest/gtest.h>
#include <core/utils/fileVisitor.h>
#include <core/cartridge.h>
#include <memory>

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
        std::filesystem::path rootPath = GBEmulator::Utils::GetRootPath();

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

    class DefaultTest : public ::testing::Test
    {
    public:
        void SetUp() override
        {
            if (!m_cartridge || m_loadedCartridgeName != m_testRomName)
            {
                EXPECT_FALSE(m_testRomName.empty());
                std::string romPath = GBEmulatorTests::FindTestRom(m_testRomName.c_str());
                EXPECT_FALSE(romPath.empty()) << "Failed to find the rom";

                if (romPath.empty())
                    return;

                GBEmulator::Utils::FileReadVisitor visitor(romPath);
                EXPECT_TRUE(visitor.IsValid()) << "Failed to open the rom";

                if (!visitor.IsValid())
                    return;

                m_cartridge = std::make_shared<GBEmulator::Cartridge>(visitor);
                EXPECT_TRUE(m_cartridge) << "Failed to load the rom";

                m_loadedCartridgeName = m_testRomName;
            }

            if (!m_cartridge)
                return;

            m_bus.InsertCartridge(m_cartridge);
        }

    protected:
        void Run(uint16_t startAddress, uint16_t endAddress)
        {
            m_bus.SetPC(startAddress);
            GBEmulatorTests::RunTo(m_bus, endAddress);
        }

        GBEmulator::Bus m_bus;
        // Put it static to avoid to re-import it everytime
        inline static std::shared_ptr<GBEmulator::Cartridge> m_cartridge = nullptr;
        inline static std::string m_loadedCartridgeName = "";

        std::string m_testRomName;
    };
}