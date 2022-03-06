#pragma once
#include <cstdint>
#include <iomanip>
#include <cstdio>

namespace GBEmulator
{
namespace Utils
{
    template <typename Stream>
    inline void Hex(Stream& s, uint16_t v, uint8_t n, const char* prefix="", const char* suffix="")
    {
        s << std::internal << std::setfill('0');
        s << prefix << std::hex << std::uppercase << std::setw(n) << (int)v << suffix;
    }

    template <typename Stream>
    inline void Dec(Stream& s, uint16_t v, uint8_t n, const char* prefix="", const char* suffix="")
    {
        s << std::internal << std::setfill('0');
        s << prefix << std::dec << std::uppercase << std::setw(n) << (int)v << suffix;
    }

    template <typename... Args>
    inline int sprintf(char* buffer, size_t bufferCount, const char* format, Args&& ...args)
    {
#ifdef _WIN32
        return sprintf_s(buffer, bufferCount, format, std::forward<Args&&>(args)...);
#else
        (void)(&bufferCount);
        return std::sprintf(buffer, format, std::forward<Args&&>(args)...)
#endif
    }
}    
}