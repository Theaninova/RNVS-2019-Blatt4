#pragma once

#include "../wulkanat/descriptive_types.h"

/**
 * Checks if an integer value is within a specified range
 *
 * @param val:      integer value of interest
 * @param upper:    upper integer value
 * @param lower:    lower integer value
 * @return bool, true if it's in range
 *
 */
#define in_range(value, upper, lower) (upper > value && lower < value)
