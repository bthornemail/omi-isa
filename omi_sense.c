#include "omi_sense.h"
#include <string.h>

/* ── Orientation word helpers ── */

uint32_t omi_sense_set_orientation(uint32_t word, uint8_t pos, uint8_t value) {
    uint32_t shift = (uint32_t)pos * 2u;
    word &= ~(0x3u << shift);
    word |= ((uint32_t)(value & 0x3u) << shift);
    return word;
}

uint8_t omi_sense_get_orientation(uint32_t word, uint8_t pos) {
    uint32_t shift = (uint32_t)pos * 2u;
    return (uint8_t)((word >> shift) & 0x3u);
}

/* ── Bitwise transduction ── */

uint16_t omi_sense_rotl16(uint16_t x, unsigned n) {
    n &= 15u;
    return (uint16_t)((x << n) | (x >> (16u - n)));
}

uint16_t omi_sense_rotr16(uint16_t x, unsigned n) {
    n &= 15u;
    return (uint16_t)((x >> n) | (x << (16u - n)));
}

uint16_t omi_sense_delta16(uint16_t x, uint16_t C) {
    return (uint16_t)(
        omi_sense_rotl16(x, 1) ^
        omi_sense_rotl16(x, 3) ^
        omi_sense_rotr16(x, 2) ^
        C
    );
}

uint16_t omi_sense_sample16(uint32_t raw, uint16_t channel_id) {
    uint16_t lo = (uint16_t)(raw & 0xFFFFu);
    uint16_t hi = (uint16_t)((raw >> 16) & 0xFFFFu);
    return (uint16_t)(lo ^ omi_sense_rotl16(hi, 5) ^ channel_id);
}

uint32_t omi_sense_pair16(uint16_t a, uint16_t b) {
    return ((uint32_t)a << 16) | (uint32_t)b;
}

uint32_t omi_sense_omi_witness(uint16_t sample, uint16_t C) {
    uint16_t d = omi_sense_delta16(sample, C);
    return ((uint32_t)sample << 16) | d;
}

uint32_t omi_sense_imo_witness(uint16_t sample, uint16_t C) {
    uint16_t d = omi_sense_delta16(sample, C);
    return ((uint32_t)d << 16) | sample;
}

/* ── Envelope bridge ──
   Light Garden Frame -> OMI 512-bit envelope layout:

   gauge[0..7]      = preheader (canonical OMI)
   orientation[0..3] = cycle (LE32)
   orientation[4..7] = source_id (LE32)
   recovery[0..3]    = orientation word (LE32)
   recovery[4..7]    = sensor_mask (LE32)
   target[0..3]      = geometry_mask (LE32)
   target[4..7]      = sample_word (LE32)
   relation[0..3]    = delta_word (LE32)
   relation[4..7]    = omi_witness (LE32)
   relation[8..11]   = imo_witness (LE32)
   relation[12..15]  = flags (LE32)
*/

static void write_le32(uint8_t* dst, uint32_t val) {
    dst[0] = (uint8_t)(val & 0xFF);
    dst[1] = (uint8_t)((val >> 8) & 0xFF);
    dst[2] = (uint8_t)((val >> 16) & 0xFF);
    dst[3] = (uint8_t)((val >> 24) & 0xFF);
}

static uint32_t read_le32(const uint8_t* src) {
    return (uint32_t)src[0]
         | ((uint32_t)src[1] << 8)
         | ((uint32_t)src[2] << 16)
         | ((uint32_t)src[3] << 24);
}

int omi_sense_frame_to_env(const OMI_SenseFrame* frame, OMI_512_Envelope* env) {
    if (!frame || !env) return -1;

    memset(env, 0, sizeof(OMI_512_Envelope));
    memcpy(env->gauge, frame->preheader, 8);
    env->gauge[1] = OMI_SENSE_MARKER;

    write_le32(env->orientation,    frame->cycle);
    write_le32(env->orientation + 4, frame->source_id);
    write_le32(env->recovery,        frame->orientation);
    write_le32(env->recovery + 4,    frame->sensor_mask);
    write_le32(env->target,          frame->geometry_mask);
    write_le32(env->target + 4,      frame->sample_word);
    write_le32(env->relation,        frame->delta_word);
    write_le32(env->relation + 4,    frame->omi_witness);
    write_le32(env->relation + 8,    frame->imo_witness);
    write_le32(env->relation + 12,   frame->flags);

    return 0;
}

int omi_sense_env_to_frame(const OMI_512_Envelope* env, OMI_SenseFrame* frame) {
    if (!env || !frame) return -1;

    memset(frame, 0, sizeof(OMI_SenseFrame));
    memcpy(frame->preheader, env->gauge, 8);

    frame->cycle         = read_le32(env->orientation);
    frame->source_id     = read_le32(env->orientation + 4);
    frame->orientation   = read_le32(env->recovery);
    frame->sensor_mask   = read_le32(env->recovery + 4);
    frame->geometry_mask = read_le32(env->target);
    frame->sample_word   = read_le32(env->target + 4);
    frame->delta_word    = read_le32(env->relation);
    frame->omi_witness   = read_le32(env->relation + 4);
    frame->imo_witness   = read_le32(env->relation + 8);
    frame->flags         = read_le32(env->relation + 12);

    return 0;
}

/* ── Frame validation (does NOT accept state) ── */
int omi_sense_validate_frame(const OMI_SenseFrame* frame) {
    if (!frame) return -1;

    const uint8_t canonical[8] = OMI_SENSE_CANONICAL_PREHEADER;
    if (memcmp(frame->preheader, canonical, 8) != 0) return -2;

    if (frame->sensor_mask == 0) return -3;
    if (frame->geometry_mask == 0) return -4;

    for (int i = 0; i < 11; i++) {
        uint8_t v = omi_sense_get_orientation(frame->orientation, (uint8_t)i);
        if (v > 3) return -5;
    }

    return 0;
}
