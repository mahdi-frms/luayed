#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define DEBUG_INFO_TYPE_NORMAL 0b00
#define DEBUG_INFO_TYPE_NUMFOR 0b01
#define DEBUG_INFO_TYPE_GENFOR 0b10

#define DEBUG_INFO_GET_TYPE(D) (D & 0xc0000000)
#define DEBUG_INFO_GET_LINE(D) ((D & 0x41111111) >> 30)

typedef uint32_t dbginfo_t;

#endif