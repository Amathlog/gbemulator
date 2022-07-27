#include <gtest/gtest.h>
#include <core/bus.h>
#include <core/cartridge.h>
#include <core/utils/fileVisitor.h>
#include <common.h>

class LDTest : public ::testing::Test
{
public:
    void SetUp()
    {
        if (!m_cartridge)
        {
            std::string romPath = GBEmulatorTests::FindTestRom("ld_test.gb");
            EXPECT_FALSE(romPath.empty()) << "Failed to find the rom";

            if (romPath.empty())
                return;

            GBEmulator::Utils::FileReadVisitor visitor(romPath);
            EXPECT_TRUE(visitor.IsValid()) << "Failed to open the rom";

            if (!visitor.IsValid())
                return;

            m_cartridge = std::make_shared<GBEmulator::Cartridge>(visitor);
            EXPECT_TRUE(m_cartridge) << "Failed to load the rom";
        }

        if (!m_cartridge)
            return;

        m_bus.InsertCartridge(m_cartridge);
    }

protected:
    void CheckSequentially(std::array<uint8_t, 7> data)
    {
        EXPECT_EQ(m_bus.GetCPU().GetBCRegister().B, data[0]);
        EXPECT_EQ(m_bus.GetCPU().GetBCRegister().C, data[1]);
        EXPECT_EQ(m_bus.GetCPU().GetDERegister().D, data[2]);
        EXPECT_EQ(m_bus.GetCPU().GetDERegister().E, data[3]);
        EXPECT_EQ(m_bus.GetCPU().GetHLRegister().H, data[4]);
        EXPECT_EQ(m_bus.GetCPU().GetHLRegister().L, data[5]);
        EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, data[6]);
    }

    void Run(uint16_t startAddress, uint16_t endAddress)
    {
        m_bus.SetPC(startAddress);
        GBEmulatorTests::RunTo(m_bus, endAddress);
    }

    GBEmulator::Bus m_bus;
    inline static std::shared_ptr<GBEmulator::Cartridge> m_cartridge = nullptr;
};

TEST_F(LDTest, LitteralLoadIn8BitsRegisters)
{
    Run(0x0200, 0x0300);
    CheckSequentially({0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE});
}

TEST_F(LDTest, LitteralLoadIn16BitsRegisters)
{
    Run(0x0300, 0x0400);

    EXPECT_EQ(m_bus.GetCPU().GetBCRegister().BC, 0xBCDE);
    EXPECT_EQ(m_bus.GetCPU().GetDERegister().DE, 0x1234);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0x5678);
    EXPECT_EQ(m_bus.GetCPU().GetStackPointer(), 0x1111);
}

TEST_F(LDTest, LitteralLoadInMemoryPointedByHL)
{
    Run(0x0400, 0x0450);

    EXPECT_EQ(m_bus.ReadByte(0xC000), 0x7F);
}

TEST_F(LDTest, LoadBIntoRegisters)
{
    Run(0x0500, 0x0600);
    CheckSequentially({0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE});
}

TEST_F(LDTest, LoadCIntoRegisters)
{
    Run(0x0600, 0x0700);
    CheckSequentially({0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF});
}

TEST_F(LDTest, LoadDIntoRegisters)
{
    Run(0x0700, 0x0800);
    CheckSequentially({0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79});
}

TEST_F(LDTest, LoadEIntoRegisters)
{
    Run(0x0800, 0x0900);
    CheckSequentially({0x14, 0x25, 0x36, 0x47, 0x58, 0x69, 0x7A});
}

TEST_F(LDTest, LoadHIntoRegisters)
{
    Run(0x0900, 0x0A00);
    CheckSequentially({0x53, 0x64, 0x75, 0x86, 0x97, 0xA8, 0xB9});
}

TEST_F(LDTest, LoadLIntoRegisters)
{
    Run(0x0A00, 0x0B00);
    CheckSequentially({0x63, 0x74, 0x85, 0x96, 0xA7, 0xB8, 0xC9});
}

TEST_F(LDTest, LoadAIntoRegisters)
{
    Run(0x0B00, 0x3FF0);
    CheckSequentially({0x73, 0x84, 0x95, 0xA6, 0xB7, 0xC8, 0xD9});
}
