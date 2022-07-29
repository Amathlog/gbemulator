#pragma once

#include <array>

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

    constexpr std::array<RGB555, 4> DEFAULT_PALETTE = { WHITE_COLOR, LIGHT_GREY_COLOR, DARK_GREY_COLOR, BLACK_COLOR };

    constexpr unsigned GB_INTERNAL_HEIGHT = 144;
    constexpr unsigned GB_INTERNAL_WIDTH = 160;
    constexpr unsigned GB_NB_PIXELS = GB_INTERNAL_HEIGHT * GB_INTERNAL_WIDTH;

    // Special addresses
    constexpr uint16_t IF_REG_ADDR = 0xFF0F;
    constexpr uint16_t IE_REG_ADDR = 0xFFFF;

    // CPU data
    constexpr size_t CPU_SINGLE_SPEED_FREQ = 4194304; // 4.194304 MHz
    constexpr size_t CPU_DOUBLE_SPEED_FREQ = 8388608; // 8.388608 MHz
}