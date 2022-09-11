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
            uint8_t lengthEnable : 1;
            uint8_t initial : 1;
        };
        
        uint8_t reg = 0x00;
    };

    union PolynomialCounterRegister
    {
        struct
        {
            uint8_t ratio : 3;
            uint8_t width : 1;
            uint8_t freq : 4;
        };

        uint8_t reg = 0x00;
    };

    union WaveVolumeRegister
    {
        struct
        {
            uint8_t unused : 5;
            uint8_t volume : 2;
            uint8_t unused2 : 1;
        };

        uint8_t reg = 0x00;
    };

    union VinRegister
    {
        struct
        {
            uint8_t SO1OutputLevel : 3;
            uint8_t outputVinToSO1 : 1;
            uint8_t SO2OutputLevel : 3;
            uint8_t outputVinToSO2 : 1;
        };

        uint8_t reg = 0x00;
    };

    union OutputTerminalRegister
    {
        struct
        {
            uint8_t channel1ToSO1 : 1;
            uint8_t channel2ToSO1 : 1;
            uint8_t channel3ToSO1 : 1;
            uint8_t channel4ToSO1 : 1;
            uint8_t channel1ToSO2 : 1;
            uint8_t channel2ToSO2 : 1;
            uint8_t channel3ToSO2 : 1;
            uint8_t channel4ToSO2 : 1;
        };

        uint8_t reg = 0x00;
    };
}