#pragma once

#include <core/mappers/mapperBase.h>
#include <core/mappers/nullMapper.h>
#include <core/mappers/mbc1.h>
#include <core/mappers/mbc2.h>
#include <core/mappers/mbc3.h>
#include <core/mappers/mbc5.h>
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
        case 0x05:
        case 0x06:
            // MBC2
            return new MBC2(header);
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return new MBC3(header);
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            return new MBC5(header);
        default:
            assert(false && "Unknown mapper");
            break;
        }

        return nullptr;
    }
}