#pragma once

#include <core/mappers/mapperBase.h>

namespace GBEmulator
{
    class MBC5 : public MapperBase
    {
    public:
        MBC5(const Header& header) : MapperBase(header) {};

        bool WriteByte(uint16_t addr, uint8_t data) override;
    };
}