#pragma once

#include <stdio.h>

// #define DEBUG to enable logging
// #define DEBUG

#define DEBUGGABLE_MAIN(argc, argv) int main(int argc, char *argv[]) { PREPARE_DEBUG();

#ifdef DEBUG
#define PREPARE_DEBUG() setbuf(stdout, 0)
#else
#define PREPARE_DEBUG()
#endif

#ifdef DEBUG
#define LOG(msg) printf("[LOG]:   %s\n", msg)
#else
#define LOG(msg)
#endif

#ifdef DEBUG
#define LOG_STR(value) printf("[VAR]:   (STR) "#value"='%s'\n", value)
#else
#define LOG_STR(msg)
#endif

#ifdef DEBUG
#define LOG_INT(value) printf("[VAR]:   (STR) "#value"='%d'\n", value)
#else
#define LOG_INT(msg)
#endif

#ifdef DEBUG
#define ERROR(msg) printf("[ERROR]: %s\n", msg)
#else
#define ERROR(msg)
#endif

#ifdef DEBUG
#define WARN(msg) printf("[WARN]:  %s\n", msg)
#else
#define WARN(msg)
#endif

#ifdef DEBUG
#define THROW(code) return code;
#else
#define THROW(code) return 0;
#endif

#ifdef DEBUG
#define HEX_VALUE_LOG(value, length) \
if (value != NULL) { \
printf("[VAR]:   (HEX) "#value"='"); \
for (size_t i = 0; i<length; i++) { \
    if (i > 0) printf(" "); \
    printf("%02X", ((char*)value)[i]);  \
}                             \
printf("'\n"); \
}
#else
#define HEX_VALUE_LOG(value, length)
#endif