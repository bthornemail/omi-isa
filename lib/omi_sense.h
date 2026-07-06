#ifndef OMI_SENSE_H
#define OMI_SENSE_H

#include "omienv.h"
#include <stdint.h>

#define OMI_SENSE_MARKER  0x0C
#define OMI_SENSE_CANONICAL_PREHEADER \
    { 0xFF, OMI_SENSE_MARKER, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF }
#define OMI_SENSE_EFFECT_PURE         0
#define OMI_SENSE_EFFECT_READ_ONLY    1
#define OMI_SENSE_EFFECT_LOCAL_WRITE  2
#define OMI_SENSE_EFFECT_REPO_WRITE   3
#define OMI_SENSE_EFFECT_NETWORK      4
#define OMI_SENSE_EFFECT_HARDWARE     5
#define OMI_SENSE_EFFECT_CAPTURE      6
#define OMI_SENSE_EFFECT_SECURITY     7

/* Sensor mask bits */
#define OMI_SENSE_SENSOR_LIGHT       (1u << 0)
#define OMI_SENSE_SENSOR_SOUND       (1u << 1)
#define OMI_SENSE_SENSOR_TOUCH       (1u << 2)
#define OMI_SENSE_SENSOR_GESTURE     (1u << 3)
#define OMI_SENSE_SENSOR_MOTION      (1u << 4)
#define OMI_SENSE_SENSOR_ORIENTATION (1u << 5)
#define OMI_SENSE_SENSOR_PROXIMITY   (1u << 6)
#define OMI_SENSE_SENSOR_CAMERA      (1u << 7)
#define OMI_SENSE_SENSOR_ENV         (1u << 8)
#define OMI_SENSE_SENSOR_NETWORK     (1u << 9)
#define OMI_SENSE_SENSOR_DOM         (1u << 10)
#define OMI_SENSE_SENSOR_HARDWARE    (1u << 11)

/* Geometry mask bits */
#define OMI_SENSE_GEOM_RING          (1u << 0)
#define OMI_SENSE_GEOM_PERSONAL      (1u << 1)
#define OMI_SENSE_GEOM_GARDEN        (1u << 2)
#define OMI_SENSE_GEOM_RADIAL        (1u << 3)
#define OMI_SENSE_GEOM_TETRAHEDRAL   (1u << 4)
#define OMI_SENSE_GEOM_5CELL         (1u << 5)
#define OMI_SENSE_GEOM_11CELL        (1u << 6)
#define OMI_SENSE_GEOM_ORBITAL       (1u << 7)

/* Light Garden Frame — raw transducer output */
typedef struct {
    uint8_t  preheader[8];
    uint32_t cycle;
    uint32_t source_id;
    uint32_t sensor_mask;
    uint32_t geometry_mask;
    uint32_t orientation;
    uint32_t sample_word;
    uint32_t delta_word;
    uint32_t omi_witness;
    uint32_t imo_witness;
    uint32_t flags;
} OMI_SenseFrame;

/* Orientation helpers */
uint32_t omi_sense_set_orientation(uint32_t word, uint8_t pos, uint8_t value);
uint8_t  omi_sense_get_orientation(uint32_t word, uint8_t pos);

/* Bitwise transduction (mirrors Light Garden spec) */
uint16_t omi_sense_rotl16(uint16_t x, unsigned n);
uint16_t omi_sense_rotr16(uint16_t x, unsigned n);
uint16_t omi_sense_delta16(uint16_t x, uint16_t C);
uint16_t omi_sense_sample16(uint32_t raw, uint16_t channel_id);
uint32_t omi_sense_pair16(uint16_t a, uint16_t b);
uint32_t omi_sense_omi_witness(uint16_t sample, uint16_t C);
uint32_t omi_sense_imo_witness(uint16_t sample, uint16_t C);

/* Envelope bridge */
int omi_sense_frame_to_env(const OMI_SenseFrame* frame, OMI_512_Envelope* env);
int omi_sense_env_to_frame(const OMI_512_Envelope* env, OMI_SenseFrame* frame);

/* Validate a candidate frame (does NOT accept state) */
int omi_sense_validate_frame(const OMI_SenseFrame* frame);

#endif
