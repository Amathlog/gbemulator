#include <gtest/gtest.h>
#include <core/bus.h>
#include <core/cartridge.h>
#include <core/utils/fileVisitor.h>
#include <common.h>

class LDTest : public GBEmulatorTests::DefaultTest
{
public:
    LDTest() : GBEmulatorTests::DefaultTest() 
    {
        m_testRomName = "ld_test.gb";
    }

protected:
    void CheckSequentially(std::array<uint8_t, 8> data)
    {
        EXPECT_EQ(m_bus.GetCPU().GetBCRegister().B, data[0]);
        EXPECT_EQ(m_bus.GetCPU().GetBCRegister().C, data[1]);
        EXPECT_EQ(m_bus.GetCPU().GetDERegister().D, data[2]);
        EXPECT_EQ(m_bus.GetCPU().GetDERegister().E, data[3]);
        EXPECT_EQ(m_bus.GetCPU().GetHLRegister().H, data[4]);
        EXPECT_EQ(m_bus.GetCPU().GetHLRegister().L, data[5]);
        EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, data[6]);
        EXPECT_EQ(m_bus.GetCPU().GetAFRegister().F.flags, data[7]);
    }
};

TEST_F(LDTest, LitteralLoadIn8BitsRegisters)
{
    Run(0x0200, 0x0300);
    CheckSequentially({0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0x00});
}

TEST_F(LDTest, LitteralLoadIn16BitsRegisters)
{
    Run(0x0300, 0x0400);

    EXPECT_EQ(m_bus.GetCPU().GetBCRegister().BC, 0xBCDE);
    EXPECT_EQ(m_bus.GetCPU().GetDERegister().DE, 0x1234);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0x5678);
    EXPECT_EQ(m_bus.GetCPU().GetStackPointer(), 0x1111);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().F.flags, 0x00);
}

TEST_F(LDTest, LitteralLoadInMemoryPointedByHL)
{
    Run(0x0400, 0x0450);

    EXPECT_EQ(m_bus.ReadByte(0xC000), 0x7F);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().F.flags, 0x00);
}

TEST_F(LDTest, LoadBIntoRegisters)
{
    Run(0x0500, 0x0600);
    CheckSequentially({0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0x00});
}

TEST_F(LDTest, LoadCIntoRegisters)
{
    Run(0x0600, 0x0700);
    CheckSequentially({0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x00});
}

TEST_F(LDTest, LoadDIntoRegisters)
{
    Run(0x0700, 0x0800);
    CheckSequentially({0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0x00});
}

TEST_F(LDTest, LoadEIntoRegisters)
{
    Run(0x0800, 0x0900);
    CheckSequentially({0x14, 0x25, 0x36, 0x47, 0x58, 0x69, 0x7A, 0x00});
}

TEST_F(LDTest, LoadHIntoRegisters)
{
    Run(0x0900, 0x0A00);
    CheckSequentially({0x53, 0x64, 0x75, 0x86, 0x97, 0xA8, 0xB9, 0x00});
}

TEST_F(LDTest, LoadLIntoRegisters)
{
    Run(0x0A00, 0x0B00);
    CheckSequentially({0x63, 0x74, 0x85, 0x96, 0xA7, 0xB8, 0xC9, 0x00});
}

TEST_F(LDTest, LoadAIntoRegisters)
{
    Run(0x0B00, 0x0C00);
    CheckSequentially({0x73, 0x84, 0x95, 0xA6, 0xB7, 0xC8, 0xD9, 0x00});
}

TEST_F(LDTest, LoadAFromMemoryRegisters)
{
    // Setup
    Run(0x0C00, 0x0C30);

    // Then run each instruction sequentially
    // ld a, [bc]
    GBEmulatorTests::RunTo(m_bus, 0x0C31);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0x12);
    EXPECT_EQ(m_bus.GetCPU().GetBCRegister().BC, 0xC080);

    // ld a, [de]
    GBEmulatorTests::RunTo(m_bus, 0x0C32);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0x34);
    EXPECT_EQ(m_bus.GetCPU().GetDERegister().DE, 0xC002);

    // ld a, [hl+]
    GBEmulatorTests::RunTo(m_bus, 0x0C33);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0x45);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0xC004);

    // ld a, [hl-]
    GBEmulatorTests::RunTo(m_bus, 0x0C34);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0x67);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0xC003);

    // ld a, [$FF00 + c]
    GBEmulatorTests::RunTo(m_bus, 0x0C35);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0xAB);
    EXPECT_EQ(m_bus.GetCPU().GetBCRegister().C, 0x80);

    // ldh a, [$FF00 + n8]
    GBEmulatorTests::RunTo(m_bus, 0x0C37);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0xCD);

    // ld a, [n16]
    GBEmulatorTests::RunTo(m_bus, 0x0C3A);
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().A, 0xAB);
}

TEST_F(LDTest, LoadAToMemoryRegisters)
{
    Run(0x0D00, 0x0E00);

    // Verify registers values
    EXPECT_EQ(m_bus.GetCPU().GetBCRegister().BC, 0xC080);
    EXPECT_EQ(m_bus.GetCPU().GetDERegister().DE, 0xC000);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0xC001);
    // Verify RAM values
    EXPECT_EQ(m_bus.ReadByte(0xC080), 0x12); // ld [bc], a
    EXPECT_EQ(m_bus.ReadByte(0xC000), 0x34); // ld [de], a
    EXPECT_EQ(m_bus.ReadByte(0xC001), 0x56); // ld [hl+], a
    EXPECT_EQ(m_bus.ReadByte(0xC002), 0x78); // ld [hl-], a
    EXPECT_EQ(m_bus.ReadByte(0xFF80), 0x9A); // ld [$FF00 + c], a
    EXPECT_EQ(m_bus.ReadByte(0xFF81), 0xBC); // ld [$FF00 + n8], a
    EXPECT_EQ(m_bus.ReadByte(0xFF82), 0xDE); // ld [n16], a
}

TEST_F(LDTest, SpecialLoads_SP)
{
    Run(0x0E00, 0x0E09);
    EXPECT_EQ(m_bus.GetCPU().GetStackPointer(), 0x9FFF); // ld sp, hl
    EXPECT_EQ(m_bus.ReadByte(0xC000), 0xFF); // ld [$C000], sp
    EXPECT_EQ(m_bus.ReadByte(0xC001), 0x9F); // ld [$C000], sp
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0x9FFF + 10); // ld hl, sp + e8
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().F.flags, 0x30); // H and C set

    GBEmulatorTests::RunTo(m_bus, 0x3FF0);
    EXPECT_EQ(m_bus.GetCPU().GetHLRegister().HL, 0x9FFF - 10); // ld hl, sp + e8
    EXPECT_EQ(m_bus.GetCPU().GetAFRegister().F.flags, 0x00);
}