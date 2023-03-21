#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define DEBUG_INFO_TYPE_NORMAL 0b00
#define DEBUG_INFO_TYPE_NUMFOR 0b01
#define DEBUG_INFO_TYPE_GENFOR 0b10

#define DEBUG_INFO_GET_TYPE(D) ((D & 0xc0000000) >> 30)
#define DEBUG_INFO_GET_LINE(D) (D & 0x3fffffff)

#define DEBUG_INFO(T, L) ((T << 30) + L)

typedef uint32_t dbginfo_t;

#endif