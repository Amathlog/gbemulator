#pragma once

#include <cstring>

#if defined(__clang__) && (__clang_major__ >= 10)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#elif defined(_MSC_VER)
#pragma warning( push )
#pragma warning ( disable : 4068 4267 4244 4305 4018 )
#endif

#include <Tonic.h>
#include "nesFilter.h"
#include "nesNoise.h"

#if defined(__clang__) && (__clang_major__ >= 10)
#pragma clang diagnostic pop
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning( pop )
#endif
