#ifndef OMI_LOADER_H
#define OMI_LOADER_H

#include <stddef.h>

#define GAUGE_BYTE_MIN  0xF0u
#define GAUGE_BYTE_MAX  0xFFu
#define GAUGE_CANONICAL 0xFFu

#define GAUGE_NUL  0x00u
#define GAUGE_FS   0x1Cu
#define GAUGE_GS   0x1Du
#define GAUGE_RS   0x1Eu
#define GAUGE_US   0x1Fu
#define GAUGE_SP   0x20u

#define PREHEADER_SIZE 8

const char* loader_strip(const char* src, size_t len, size_t* payload_len);

#endif
