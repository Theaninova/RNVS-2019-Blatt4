#pragma once

#include <stdlib.h>

#define size(name) (*(((size_t*) name) - sizeof(size_t)))

#define heap(name, amount) \
*name = (calloc(amount + sizeof(size_t), 1) + sizeof(size_t)); \
size(name) = amount

#define dealloc(name) free(name - sizeof(size_t))

#define stack(name, amount) name[amount]
