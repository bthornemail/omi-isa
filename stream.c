#include "stream.h"
#include "omi_dispatch.h"
#include <string.h>

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

void stream_parser_init(StreamParser* sp) {
    if (!sp) return;
    memset(sp, 0, sizeof(StreamParser));
    sp->state = STREAM_STATE_WAITING;
    sp->filled = 0;
    sp->gauge_match_count = 0;
    sp->validation_result = -1;
}

void stream_push_byte(StreamParser* sp, uint8_t byte) {
    if (!sp) return;
    switch (sp->state) {
        case STREAM_STATE_WAITING:
            if (byte == 0xFF) {
                sp->buffer[0] = byte;
                sp->filled = 1;
                sp->gauge_match_count = 1;
                sp->state = STREAM_STATE_HEADER;
            }
            break;
        case STREAM_STATE_HEADER:
            if (sp->filled < 8) {
                sp->buffer[sp->filled++] = byte;
                if (byte == CANONICAL_PREHEADER[sp->filled - 1])
                    sp->gauge_match_count++;
                else {
                    if (byte == 0xFF) {
                        sp->buffer[0] = byte;
                        sp->filled = 1;
                        sp->gauge_match_count = 1;
                    } else {
                        sp->state = STREAM_STATE_WAITING;
                        sp->filled = 0;
                        sp->gauge_match_count = 0;
                    }
                    return;
                }
                if (sp->filled == 8)
                    sp->state = (sp->gauge_match_count == 8)
                        ? STREAM_STATE_PAYLOAD : STREAM_STATE_INVALID;
            }
            break;
        case STREAM_STATE_PAYLOAD:
            if (sp->filled < OMI_ENV_SIZE) {
                sp->buffer[sp->filled++] = byte;
                if (sp->filled == OMI_ENV_SIZE) {
                    memcpy(&sp->envelope, sp->buffer, OMI_ENV_SIZE);
                    sp->validation_result = omi_env_validate(&sp->envelope);
                    if (sp->validation_result == 0) {
                        sp->state = STREAM_STATE_COMPLETE;
                        omi_env_to_bitboard(&sp->envelope, &sp->bitboard);
                        if (sp->auto_dispatch) {
                            sp->dispatch_result = stream_execute(sp);
                        }
                    } else {
                        sp->state = STREAM_STATE_INVALID;
                    }
                }
            }
            break;
        case STREAM_STATE_COMPLETE:
        case STREAM_STATE_INVALID:
            break;
    }
}

int stream_has_event(const StreamParser* sp) {
    if (!sp) return 0;
    return (sp->state == STREAM_STATE_COMPLETE);
}

int stream_pop_event(StreamParser* sp, StreamEvent* evt) {
    if (!sp || !evt) return -1;
    if (sp->state != STREAM_STATE_COMPLETE) return -2;
    evt->envelope = sp->envelope;
    evt->bitboard = sp->bitboard;
    evt->valid = (sp->validation_result == 0);
    evt->cell_index = 0;
    evt->opcode = omi_env_get_opcode(&sp->envelope);
    evt->dispatch_result = sp->dispatch_result;
    evt->response_len = sp->response_len;
    if (sp->response_len > 0)
        memcpy(evt->response, sp->response_buffer, sp->response_len);
    stream_parser_init(sp);
    return 0;
}

int stream_execute(StreamParser* sp) {
    if (!sp) return -1;
    if (sp->state != STREAM_STATE_COMPLETE) return -2;
    if (!sp->vm) return -3;

    OMI_DispatchContext ctx;
    ctx.env = &sp->envelope;
    ctx.bitboard = sp->bitboard;
    ctx.vm = sp->vm;
    ctx.tx_buffer = sp->response_buffer;
    ctx.tx_capacity = OMI_ENV_SIZE;
    ctx.tx_len = &sp->response_len;
    ctx.transport_ctx = NULL;

    sp->response_len = 0;
    int result = omi_dispatch_execute(&ctx);
    sp->dispatch_result = result;
    return result;
}

static void build_binary_envelope(OMI_512_Envelope* env,
    uint8_t orient0, uint8_t orient1,
    const char* label, uint32_t value)
{
    memset(env, 0, sizeof(OMI_512_Envelope));
    memcpy(env->gauge, CANONICAL_PREHEADER, 8);
    env->orientation[0] = orient0;
    env->orientation[1] = orient1;
    env->relation[0] = (uint8_t)((value >> 24) & 0xFF);
    env->relation[1] = (uint8_t)((value >> 16) & 0xFF);
    env->relation[2] = (uint8_t)((value >> 8) & 0xFF);
    env->relation[3] = (uint8_t)(value & 0xFF);
    if (label) {
        size_t llen = strlen(label);
        if (llen > 16) llen = 16;
        memcpy(env->relation + 16, label, llen);
    }
}

int stream_inject_sensor(StreamParser* sp, int pin, int value) {
    if (!sp) return -1;
    OMI_512_Envelope env;
    build_binary_envelope(&env, (uint8_t)(pin & 0xFF), (uint8_t)(value & 0xFF),
        "gpio", (uint32_t)value);
    uint8_t* bytes = (uint8_t*)&env;
    for (size_t i = 0; i < OMI_ENV_SIZE; i++)
        stream_push_byte(sp, bytes[i]);
    return 0;
}

int stream_inject_event(StreamParser* sp, const char* event_type, uint32_t value) {
    if (!sp || !event_type) return -1;
    OMI_512_Envelope env;
    build_binary_envelope(&env, (uint8_t)(strlen(event_type) & 0xFF), 1,
        event_type, value);
    uint8_t* bytes = (uint8_t*)&env;
    for (size_t i = 0; i < OMI_ENV_SIZE; i++)
        stream_push_byte(sp, bytes[i]);
    return 0;
}

int stream_inject_hardware(StreamParser* sp, const char* device, const char* action) {
    if (!sp || !device || !action) return -1;
    OMI_512_Envelope env;
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, CANONICAL_PREHEADER, 8);
    size_t dlen = strlen(device);
    if (dlen > 8) dlen = 8;
    memcpy(env.orientation, device, dlen);
    size_t alen = strlen(action);
    if (alen > 8) alen = 8;
    memcpy(env.target, action, alen);
    env.target[0] = 2;
    uint8_t* bytes = (uint8_t*)&env;
    for (size_t i = 0; i < OMI_ENV_SIZE; i++)
        stream_push_byte(sp, bytes[i]);
    return 0;
}
