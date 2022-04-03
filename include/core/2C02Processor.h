#pragma once

#include <core/serializable.h>

namespace GBEmulator
{
    union LCDRegister
    {
        struct
        {
            uint8_t enable : 1;
            uint8_t windowTileMapArea : 1;
            uint8_t windowEnable : 1;
            uint8_t BGAndWindowTileAreaData : 1;
            uint8_t bgTileMapArea : 1;
            uint8_t objSize : 1;
            uint8_t objEnable : 1;
            uint8_t BGAndWindowPriority : 1;
        };

        uint8_t flags = 0x00;
    };


    class Processor2C02 : public ISerializable
    {
    public:
        uint8_t ReadByte(uint16_t addr);
        void WriteByte(uint16_t addr, uint8_t data);

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

    private:
        LCDRegister m_lcdRegister;
    };
}