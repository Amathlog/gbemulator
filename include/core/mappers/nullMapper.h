#pragma once

#include <core/mappers/mapperBase.h>
#include <cstdint>

namespace GBEmulator
{
    class NullMapper : public MapperBase
    {
    public:
        NullMapper(const Header& header) : MapperBase(header) 
        {
            // Enable the RAM in the NullMapper if we ever have some RAM
            // But it will be stuck to the first bank.
            if (header.nbRamBanks > 0)
            {
                m_ramEnabled = true;
            }
        }

        virtual bool WriteByte(uint16_t addr, uint8_t data){ return false; };
    };
}