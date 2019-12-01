#pragma once

#include <stdint.h>
#include "malloc.h"

// type system
typedef void (*Destructor)(void *self);

#define __SMART_POINTER_DESTRUCTOR_NAME(type) type##_smart_pointer_destructor

#define DEFINE_SMART_POINTER_OF_TYPE(type) \
typedef struct { \
    type *value; \
    size_t length; \
    uint16_t references; \
    Destructor destructor; \
} type##_smart_pointer; \
void __SMART_POINTER_DESTRUCTOR_NAME(type)(type *self)

// smart pointer types
DEFINE_SMART_POINTER_OF_TYPE(void) {}

// destructors
#define DESTRUCTOR(struct_name, name) void __destruct_##struct_name(struct_name *name)
#define DESTRUCTOR_NAME(struct_name) (Destructor) __destruct_##struct_name

// for use in functions etc
#define __SP(name) _##name
#define SMART_POINTER(type, name) __SP_TYPE_NAME(type)* __SP(name)
#define SMART_POINTER_RETURN_TYPE(type) __SP_TYPE_NAME(type)*

#define it(name) (*(__SP(name)->value))
#define as(name, type) (*((type*) __SP(name)->value))

// internal functions
#define __GENERIC_SMART_POINTER __SP_TYPE_NAME(void)
#define __SP_TYPE_NAME(type) type##_smart_pointer

// when calling functions
#define PASS(name) __SP(name)

#define SMART_FREE(name) \
if (__SP(name) != NULL && --(__SP(name))->references == 0) { \
    if (__SP(name)->destructor != NULL) { __SP(name)->destructor(__SP(name)->value); } \
    free(__SP(name)); \
}

#define USE(name, ...) \
do { \
    typeof(__SP(name)->value) name = __SP(name)->value; \
    __SP(name)->references++; \
    do __VA_ARGS__ while(0); \
    SMART_FREE(name) \
} while (0);

#define FROM_STRUCT_USE(strct, name, new_name, ...) \
do { \
    typeof(strct->__SP(name)) __SP(new_name) = strct->__SP(name); \
    USE(new_name, __VA_ARGS__) \
    } while (0);

#define FROM_STACK_STRUCT_USE(strct, name, new_name, ...) \
do { \
    typeof(strct.__SP(name)) __SP(new_name) = strct.__SP(name); \
    USE(new_name, __VA_ARGS__) \
    } while (0);

#define ABANDON(from) \
SMART_FREE(from) \
__SP(from) = NULL;

#define MALLOC_ASSIGN(type, ptr, value_length) \
do { \
    if (__SP(ptr) != NULL) {ABANDON(ptr)} \
    __SP(ptr) = malloc(sizeof(__GENERIC_SMART_POINTER) + value_length); \
    __SP(ptr)->value = (type*) __SP(ptr) + sizeof(__GENERIC_SMART_POINTER); \
    __SP(ptr)->length = value_length; \
    __SP(ptr)->references = 1; /*because we assign it, we already have a reference*/\
    __SP(ptr)->destructor = (Destructor) __SMART_POINTER_DESTRUCTOR_NAME(type); \
} while (0);

#define MALLOC_COPY(type, name, from, length) \
do { \
    MALLOC_ASSIGN(type, name, length) \
    memcpy(__SP(name), from, length); \
} while (0);

#define MALLOC(type, name, value_length, ...) \
do { \
    SMART_POINTER(type, name) = NULL; \
    MALLOC_ASSIGN(type, name, value_length) \
    USE(name, __VA_ARGS__) \
    SMART_FREE(name)\
} while (0);

#define ASSIGN(to, name) \
if (__SP(to) != NULL) {ABANDON(to)} \
__SP(name)->references++; \
__SP(to) = __SP(name);

#define SELF_ABANDON(self, key) \
FROM_STRUCT_USE(self, key, self_key, { \
    ABANDON(self_key) \
})

#define USE_NULL(type, name, ...) \
do { \
SMART_POINTER(type, name) = NULL; \
USE(name, __VA_ARGS__) \
} while (0);

#define RETURN(name) return __SP(name);

// EXAMPLES
/*
SMART_POINTER(void, some_global);

void test2(SMART_POINTER(void, param1)) {

    USE(param1, {
        ASSIGN(some_global, param1)
    })
}

void test() {
    SMART_POINTER(void, name);

    MALLOC(void, pointer, 10, {
        test2(PASS(pointer));
    })

    ABANDON(some_global)
}*/
