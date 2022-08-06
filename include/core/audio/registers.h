#pragma once

#include <cstdint>

namespace GBEmulator
{
    union SweepRegister
    {
        struct
        {
            uint8_t shift : 3;
            uint8_t decrease : 1;
            uint8_t time : 3;
            uint8_t unused : 1;
        };

        uint8_t reg = 0x00;
    };

    union WavePatternRegister
    {
        struct
        {
            uint8_t length : 6;
            uint8_t duty : 2;
        };

        uint8_t reg = 0x00;
    };

    union VolumeEnveloppeRegister
    {
        struct
        {
            uint8_t nbEnveloppeSweep : 3;
            uint8_t enveloppeDirection : 1;
            uint8_t initialVolume : 4;
        };

        uint8_t reg = 0x00;
    };

    union FrequencyHighRegister
    {
        struct
        {
            uint8_t freqMsb : 3;
            uint8_t unused : 3;
            uint8_t counterConsecutiveSelection : 1;
            uint8_t initial : 1;
        };
        
        uint8_t reg = 0x00;
    };
}