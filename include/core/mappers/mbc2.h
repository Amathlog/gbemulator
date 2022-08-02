#pragma once

#include <core/mappers/mapperBase.h>

namespace GBEmulator
{
    class MBC2 : public MapperBase
    {
    public:
        MBC2(const Header& header);
        bool WriteByte(uint16_t addr, uint8_t data) override;

        size_t GetRAMSize() const override { return 0x200; }
    };
}