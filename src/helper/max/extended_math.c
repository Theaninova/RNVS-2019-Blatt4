#include "extended_math.h"

bool in_range(byte32 value, byte32 upper, byte32 lower) {
    return (upper >= value && lower <= value);
}
