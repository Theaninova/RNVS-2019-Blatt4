#pragma once

#include <stdint.h>

#define int8 int8_t
#define int16 int16_t
#define int32 int32_t
#define int64 int64_t
#define int128 int128_t

#define byte8 uint8_t
#define byte16 uint16_t
#define byte32 uint32_t
#define byte64 uint64_t
#define byte128 uint128_t

#define float32 float
#define float64 double
#define float128 long double

#define unknown void

#define bool byte8
#define true 1
#define false 0
#define truthy(value) (value ? true : false)

#define string char*

#define val const
