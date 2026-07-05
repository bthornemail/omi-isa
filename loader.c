#include "loader.h"
#include <string.h>

const char* loader_strip(const char* src, size_t len, size_t* payload_len){
    if(!src || !payload_len){ *payload_len = 0; return src; }

    // Binary pre-header: FF 00 1C 1D 1E 1F 20 FF
    static const unsigned char canonical[PREHEADER_SIZE] = {
        GAUGE_CANONICAL, GAUGE_NUL, GAUGE_FS, GAUGE_GS,
        GAUGE_RS, GAUGE_US, GAUGE_SP, GAUGE_CANONICAL
    };

    if(len >= PREHEADER_SIZE && memcmp(src, canonical, PREHEADER_SIZE) == 0){
        *payload_len = len - PREHEADER_SIZE;
        return src + PREHEADER_SIZE;
    }

    // No carrier detected — treat entire input as raw payload
    *payload_len = len;
    return src;
}
