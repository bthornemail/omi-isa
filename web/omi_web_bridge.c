#include <emscripten.h>
#include "stream.h"
#include "omienv.h"
#include "omi_dispatch.h"
#include "cpu.h"
#include <string.h>

static StreamParser stream_parser;
static OMI_CPU cpu;
static uint8_t envelope_copy[OMI_ENV_SIZE];
static int envelope_ready;
static uint64_t last_bitboard;
static uint8_t response_copy[OMI_ENV_SIZE];
static size_t response_copy_len;

EMSCRIPTEN_KEEPALIVE
void omi_web_init(void) {
    init_cpu(&cpu);
    cpu.capability = 0xFFFFFFFF;
    stream_parser_init(&stream_parser);
    stream_parser.auto_dispatch = 1;
    stream_parser.vm = &cpu;
    omi_dispatch_init();
    omi_gauge_init();
    envelope_ready = 0;
    last_bitboard = 0;
    response_copy_len = 0;
}

EMSCRIPTEN_KEEPALIVE
void omi_web_push_byte(uint8_t byte) {
    stream_push_byte(&stream_parser, byte);
    if (stream_parser.state == STREAM_STATE_COMPLETE) {
        memcpy(envelope_copy, &stream_parser.envelope, OMI_ENV_SIZE);
        last_bitboard = stream_parser.bitboard;
        envelope_ready = 1;
        if (stream_parser.response_len > 0) {
            response_copy_len = stream_parser.response_len;
            memcpy(response_copy, stream_parser.response_buffer, stream_parser.response_len);
        }
    }
}

EMSCRIPTEN_KEEPALIVE
int omi_web_has_event(void) {
    return envelope_ready;
}

EMSCRIPTEN_KEEPALIVE
const uint8_t* omi_web_get_envelope(void) {
    return envelope_copy;
}

EMSCRIPTEN_KEEPALIVE
int omi_web_get_opcode(void) {
    OMI_512_Envelope env;
    memcpy(&env, envelope_copy, OMI_ENV_SIZE);
    return omi_env_get_opcode(&env);
}

EMSCRIPTEN_KEEPALIVE
uint64_t omi_web_get_bitboard(void) {
    return last_bitboard;
}

EMSCRIPTEN_KEEPALIVE
void omi_web_pop_event(void) {
    StreamEvent evt;
    stream_pop_event(&stream_parser, &evt);
    envelope_ready = 0;
    response_copy_len = 0;
}

EMSCRIPTEN_KEEPALIVE
int omi_web_has_response(void) {
    return response_copy_len > 0 ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
const uint8_t* omi_web_get_response(void) {
    return response_copy;
}

EMSCRIPTEN_KEEPALIVE
size_t omi_web_get_response_len(void) {
    return response_copy_len;
}

EMSCRIPTEN_KEEPALIVE
int omi_web_execute(void) {
    if (!envelope_ready) return -1;
    stream_parser.state = STREAM_STATE_COMPLETE;
    memcpy(&stream_parser.envelope, envelope_copy, OMI_ENV_SIZE);
    stream_parser.bitboard = last_bitboard;
    int r = stream_execute(&stream_parser);
    if (stream_parser.response_len > 0) {
        response_copy_len = stream_parser.response_len;
        memcpy(response_copy, stream_parser.response_buffer, stream_parser.response_len);
    }
    return r;
}

EMSCRIPTEN_KEEPALIVE
int omi_web_inject_sensor(int pin, int value) {
    return stream_inject_sensor(&stream_parser, pin, value);
}

EMSCRIPTEN_KEEPALIVE
int omi_web_inject_event(const char* event_type, uint32_t value) {
    return stream_inject_event(&stream_parser, event_type, value);
}

EMSCRIPTEN_KEEPALIVE
int omi_web_inject_hardware(const char* device, const char* action) {
    return stream_inject_hardware(&stream_parser, device, action);
}
