//
// Created by maxtreme on 01.12.19.
//

#include "../wulkanat/helper/descriptive_types.h"

#ifndef BLATT4_EXTENDED_MATH_H
#define BLATT4_EXTENDED_MATH_H

/**
 * Checks if an integer value is within a specified range
 *
 * @param val:      integer value of interest
 * @param upper:    upper integer value
 * @param lower:    lower integer value
 * @return bool, true if it's in range
 *
 */

bool is_in_range(uint32 val, uint32 upper, uint32 lower) {
    return ( upper >= val && lower <= val);
}

#endif //BLATT4_EXTENDED_MATH_H
