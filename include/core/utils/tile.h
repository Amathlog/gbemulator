#pragma once

#include <cstdint>
#include <array>
#include <core/constants.h>

namespace GBEmulator
{
namespace Utils
{
    inline std::array<uint8_t, 64> GetTileDataFromBytes(const uint8_t* data)
    {
        std::array<uint8_t, 64> res;
        for (auto i = 0; i < 8; ++i)
        {
            uint8_t lsb = data[2 * i];
            uint8_t msb = data[2 * i + 1];

            for (auto j = 0; j < 8; ++j)
            {
                res[8 * i + j] = ((msb & 0x01) << 1) | (lsb & 0x01);
                msb >>= 1;
                lsb >>= 1;
            }
        }

        return res;
    }

    constexpr inline void RGB555ToRGB888(const RGB555& colorIn, uint8_t& rOut, uint8_t& gOut, uint8_t& bOut)
    {
        rOut = colorIn.R << 3;
        gOut = colorIn.G << 3;
        bOut = colorIn.B << 3;
    }
}
}