
namespace GBEmulator
{
    union RGB555
    {
        struct
        {
            uint16_t R : 5;
            uint16_t G : 5;
            uint16_t B : 5;
            uint16_t unused : 1;
        };

        uint16_t data = 0x0000;
    };

    constexpr RGB555 WHITE_COLOR = { 0x1F, 0x1F, 0x1F, 0 };
    constexpr RGB555 LIGHT_GREY_COLOR = { 0x15, 0x15, 0x15, 0 };
    constexpr RGB555 DARK_GREY_COLOR = { 0x0A, 0x0A, 0x0A, 0 };
    constexpr RGB555 BLACK_COLOR = { 0x00, 0x00, 0x00, 0 };

    constexpr unsigned GB_INTERNAL_HEIGHT = 160;
    constexpr unsigned GB_INTERNAL_WIDTH = 144;
}