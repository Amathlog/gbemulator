#pragma once

#include <core/mappers/mapperBase.h>
#include <core/mappers/nullMapper.h>
#include <core/mappers/mbc1.h>
#include <core/cartridge.h>
#include <cassert>

namespace GBEmulator
{
    MapperBase* CreateMapper(const Header& header)
    {
        switch(header.cartridgeType)
        {
        case 0x00:
        case 0x08:
        case 0x09:
            // ROM Only
            return new NullMapper(header);

        case 0x01:
        case 0x02:
        case 0x03:
            // MBC1
            return new MBC1(header);
        default:
            assert(false && "Unknown mapper");
            break;
        }

        return nullptr;
    }
}