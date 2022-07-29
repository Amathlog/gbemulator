#include <common.h>

class ArithmeticTest : public GBEmulatorTests::DefaultTest
{
public:
    ArithmeticTest() : GBEmulatorTests::DefaultTest()
    {
        m_testRomName = "arithmetic_test.gb";
    }

protected:
    // Read on the stack, so decrement the addr instead of increment 
    void CheckFlagsAndA(uint16_t addr, uint8_t flags, uint8_t a, const char* what)
    {
        EXPECT_EQ(m_bus.ReadByte(addr), a) << what;
        EXPECT_EQ(m_bus.ReadByte(addr - 1), flags) << what;
    }
};

// PUSH AF will first decrement the SP and then load in memory
// so data starts to be written at 0xD000 - 1
constexpr uint16_t sp_start = 0xD000 - 1;

TEST_F(ArithmeticTest, AddTest)
{
    Run(0x0200, 0x0300);

    uint16_t addr = sp_start;
    
    // a = 0, b = 1 => a = 1; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x01, "add a, b"); 
    addr -= 2;

    // a = 1, c = 2 => a = 3; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x03, "add a, c");
    addr -= 2;

    // a = 3, d = 3 => a = 0x06; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x06, "add a, d");
    addr -= 2;

    // a = 6, e = 4 => a = 10; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x0A,  "add a, e");
    addr -= 2;

    // a = 10, h = 0xC0 => a = 0xCA; flags = 0x20
    CheckFlagsAndA(addr, 0x00, 0xCA, "add a, h");
    addr -= 2;

    // a = 0xCA, l = 5 => a = 0xCF; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0xCF, "add a, l");
    addr -= 2;

    // a = 0xCF, [hl] = 0xFF => a = 0xCE; flags = 0x10
    CheckFlagsAndA(addr, 0x30, 0xCE, "add a, [hl]");
    addr -= 2;

    // a = 0xCE, a = 0xCE => a = 0x9C; flags = 0x10
    CheckFlagsAndA(addr, 0x30, 0x9C, "add a, a");
    addr -= 2;

    // a = 0x9C, litteral = 10 => a = 0xA6; flags = 0x20
    CheckFlagsAndA(addr, 0x20, 0xA6, "add a, 10");
    addr -= 2;
}

TEST_F(ArithmeticTest, AdcTest)
{
    Run(0x0300, 0x0400);

    uint16_t addr = sp_start;
    
    // a = 0xff, b = 1, carry = 0 => a = 0x00; flags = 0xB0
    CheckFlagsAndA(addr, 0xB0, 0x00, "adc a, b - no carry"); 
    addr -= 2;

    // a = 0, b = 1, carry = 1 => a = 2; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x02, "adc a, b - carry");
    addr -= 2;

    // a = 0xff, c = 2, carry = 0 => a = 0x01; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0x01, "adc a, c - no carry"); 
    addr -= 2;

    // a = 1, c = 2, carry = 1 => a = 4; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x04, "adc a, c - carry");
    addr -= 2;

    // a = 0xff, d = 3, carry = 0 => a = 0x02; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0x02, "adc a, d - no carry"); 
    addr -= 2;

    // a = 2, d = 3, carry = 1 => a = 6; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x06, "adc a, d - carry");
    addr -= 2;

    // a = 0xff, e = 4, carry = 0 => a = 0x03; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0x03, "adc a, e - no carry"); 
    addr -= 2;

    // a = 3, e = 4, carry = 1 => a = 8; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x08, "adc a, e - carry");
    addr -= 2;

    // a = 0xff, h = 0xC0, carry = 0 => a = 0xBF; flags = 0x10
    CheckFlagsAndA(addr, 0x10, 0xBF, "adc a, h - no carry"); 
    addr -= 2;

    // a = 0xBF, h = 0xC0, carry = 1 => a = 0x80; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0x80, "adc a, h - carry");
    addr -= 2;

    // a = 0x80, l = 5, carry = 1 => a = 0x86; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x86, "adc a, l - carry"); 
    addr -= 2;

    // a = 0x86, l = 5, carry = 0 => a = 0x8B; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x8B, "adc a, l - no carry");
    addr -= 2;

    // a = 0xff, [hl] = 0xff, carry = 0 => a = 0xfe; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0xFE, "adc a, [hl] - no carry"); 
    addr -= 2;

    // a = 0xfe, [hl] = 0xff, carry = 1 => a = 0xfe; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0xFE, "adc a, [hl] - carry");
    addr -= 2;

    // a = 0x01, a = 0x01, carry = 1 => a = 0x03; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x03, "adc a, a - carry"); 
    addr -= 2;

    // a = 0x03, a = 0x03, carry = 0 => a = 0x06; flags = 0x00
    CheckFlagsAndA(addr, 0x00, 0x06, "adc a, a - no carry");
    addr -= 2;

    // a = 0xff, litteral = 10, carry = 0 => a = 0x09; flags = 0x30
    CheckFlagsAndA(addr, 0x30, 0x09, "adc a, 10 - no carry"); 
    addr -= 2;

    // a = 9, litteral = 10, carry = 1 => a = 20; flags = 0x20
    CheckFlagsAndA(addr, 0x20, 0x14, "adc a, 10 - carry");
    addr -= 2;
}

TEST_F(ArithmeticTest, SubTest)
{
    Run(0x0400, 0x3FF0);

    uint16_t addr = sp_start;
    
    // a = 0, b = 1 => a = 0xFF; flags = 0x70
    CheckFlagsAndA(addr, 0x70, 0xff, "sub a, b"); 
    addr -= 2;

    // a = 0xFF, c = 2 => a = 0xFD; flags = 0x40
    CheckFlagsAndA(addr, 0x40, 0xFD, "sub a, c");
    addr -= 2;

    // a = 0xFD, d = 3 => a = 0xFA; flags = 0x40
    CheckFlagsAndA(addr, 0x40, 0xFA, "sub a, d");
    addr -= 2;

    // a = 0xFA, e = 4 => a = 0xF6; flags = 0x40
    CheckFlagsAndA(addr, 0x40, 0xF6,  "sub a, e");
    addr -= 2;

    // a = 0xF6, h = 0xC0 => a = 0x36; flags = 0x40
    CheckFlagsAndA(addr, 0x40, 0x36, "sub a, h");
    addr -= 2;

    // a = 0x36, l = 5 => a = 0x31; flags = 0x40
    CheckFlagsAndA(addr, 0x40, 0x31, "sub a, l");
    addr -= 2;

    // a = 0x31, [hl] = 0xFF => a = 0x32; flags = 0x70
    CheckFlagsAndA(addr, 0x70, 0x32, "sub a, [hl]");
    addr -= 2;

    // a = 0x32, a = 0x32 => a = 0x00; flags = 0xC0
    CheckFlagsAndA(addr, 0xC0, 0x00, "sub a, a");
    addr -= 2;

    // a = 0x0, litteral = 10 => a = 0xF7; flags = 0x70
    CheckFlagsAndA(addr, 0x70, 0xF6, "sub a, 10");
    addr -= 2;
}