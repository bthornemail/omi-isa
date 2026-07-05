#include "omi_sense.h"
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_run = 0;

static void TEST(const char* name, int cond) {
    tests_run++;
    if (cond) tests_passed++;
    printf("  %s: %s\n", cond ? "PASS" : "FAIL", name);
}

int main(void) {
    printf("=== Omi-Sense / Light Garden Adapter Tests ===\n\n");

    /* ── Test 1: Bitwise operations ── */
    printf("[Test 1] Bitwise transduction\n");
    TEST("rotl16(0x0001, 1) == 0x0002",
         omi_sense_rotl16(0x0001, 1) == 0x0002);
    TEST("rotl16(0x8000, 1) == 0x0001",
         omi_sense_rotl16(0x8000, 1) == 0x0001);
    TEST("rotr16(0x0002, 1) == 0x0001",
         omi_sense_rotr16(0x0002, 1) == 0x0001);
    TEST("rotr16(0x0001, 1) == 0x8000",
         omi_sense_rotr16(0x0001, 1) == 0x8000);

    printf("\n[Test 2] delta16\n");
    uint16_t d0  = omi_sense_delta16(0, 0);
    TEST("delta16(0,0) == 0 (zero fixed point when C=0)", d0 == 0);
    uint16_t dC  = omi_sense_delta16(0, 0xABCD);
    TEST("delta16(0,C) == C (reduces to C when x=0)", dC == 0xABCD);
    uint16_t dNZ = omi_sense_delta16(0x1234, 0x5678);
    uint16_t rev = omi_sense_delta16(dNZ, 0);
    TEST("delta16 not trivially reversible", rev != dNZ);

    printf("\n[Test 3] sample16\n");
    uint16_t s1 = omi_sense_sample16(0xDEADBEEF, 0);
    uint16_t s2 = omi_sense_sample16(0xDEADBEEF, 1);
    uint16_t s3 = omi_sense_sample16(0xCAFEBABE, 0);
    TEST("sample16 same input, same channel == same output",
         s1 == omi_sense_sample16(0xDEADBEEF, 0));
    TEST("sample16 different channel -> different",
         s1 != s2);
    TEST("sample16 different input -> different",
         s1 != s3);

    printf("\n[Test 4] Orientation helpers\n");
    uint32_t word = 0;
    word = omi_sense_set_orientation(word, 0, 1);
    TEST("set_orientation(pos 0, val 1)", omi_sense_get_orientation(word, 0) == 1);
    word = omi_sense_set_orientation(word, 1, 2);
    TEST("set_orientation(pos 1, val 2)", omi_sense_get_orientation(word, 1) == 2);
    word = omi_sense_set_orientation(word, 2, 3);
    TEST("set_orientation(pos 2, val 3)", omi_sense_get_orientation(word, 2) == 3);
    TEST("pos 3 still 0",  omi_sense_get_orientation(word, 3) == 0);
    TEST("pos 10 still 0", omi_sense_get_orientation(word, 10) == 0);

    /* ── Test 5: Witness derivation ── */
    printf("\n[Test 5] OMI/IMO witnesses\n");
    uint32_t omi_w = omi_sense_omi_witness(0x1234, 0x55AA);
    uint32_t imo_w = omi_sense_imo_witness(0x1234, 0x55AA);
    TEST("omi_witness top 16 == sample",
         (omi_w >> 16) == 0x1234);
    TEST("imo_witness bottom 16 == sample",
         (imo_w & 0xFFFF) == 0x1234);
    TEST("omi_witness delta matches imo_witness delta top 16",
         (omi_w & 0xFFFF) == (imo_w >> 16));
    TEST("omi_witness != imo_witness (different ordering)",
         omi_w != imo_w);

    /* ── Test 6: Frame → Envelope round-trip ── */
    printf("\n[Test 6] Frame <-> Envelope round-trip\n");
    OMI_SenseFrame frame;
    memset(&frame, 0, sizeof(frame));
    memcpy(frame.preheader, "\xFF\x0C\x1C\x1D\x1E\x1F\x20\xFF", 8);
    frame.cycle         = 42;
    frame.source_id     = 0x0A;
    frame.sensor_mask   = OMI_SENSE_SENSOR_LIGHT | OMI_SENSE_SENSOR_SOUND;
    frame.geometry_mask = OMI_SENSE_GEOM_RING | OMI_SENSE_GEOM_PERSONAL;
    frame.orientation   = omi_sense_set_orientation(0, 0, 1);
    frame.orientation   = omi_sense_set_orientation(frame.orientation, 1, 2);
    frame.sample_word   = 0xDEADBEEF;
    frame.delta_word    = 0x12345678;
    frame.omi_witness   = omi_sense_omi_witness(0xABCD, 0x55AA);
    frame.imo_witness   = omi_sense_imo_witness(0xABCD, 0x55AA);
    frame.flags         = OMI_SENSE_EFFECT_READ_ONLY;

    OMI_512_Envelope env;
    int ret = omi_sense_frame_to_env(&frame, &env);
    TEST("frame_to_env returns 0", ret == 0);
    TEST("env gauge[1] == OMI_SENSE_MARKER",
         env.gauge[1] == OMI_SENSE_MARKER);

    OMI_SenseFrame back;
    ret = omi_sense_env_to_frame(&env, &back);
    TEST("env_to_frame returns 0", ret == 0);
    TEST("cycle round-trips",  back.cycle == 42);
    TEST("source_id round-trips", back.source_id == 0x0A);
    TEST("sensor_mask round-trips", back.sensor_mask == (OMI_SENSE_SENSOR_LIGHT | OMI_SENSE_SENSOR_SOUND));
    TEST("geometry_mask round-trips", back.geometry_mask == (OMI_SENSE_GEOM_RING | OMI_SENSE_GEOM_PERSONAL));
    TEST("orientation round-trips", back.orientation == frame.orientation);
    TEST("sample_word round-trips", back.sample_word == 0xDEADBEEF);
    TEST("delta_word round-trips", back.delta_word == 0x12345678);
    TEST("omi_witness round-trips", back.omi_witness == frame.omi_witness);
    TEST("imo_witness round-trips", back.imo_witness == frame.imo_witness);
    TEST("flags round-trips", back.flags == OMI_SENSE_EFFECT_READ_ONLY);
    TEST("preheader round-trips",
         memcmp(back.preheader, frame.preheader, 8) == 0);

    /* ── Test 7: Null safety ── */
    printf("\n[Test 7] Null safety\n");
    TEST("frame_to_env null frame", omi_sense_frame_to_env(NULL, &env) == -1);
    TEST("frame_to_env null env",   omi_sense_frame_to_env(&frame, NULL) == -1);
    TEST("env_to_frame null env",   omi_sense_env_to_frame(NULL, &back) == -1);
    TEST("env_to_frame null frame", omi_sense_env_to_frame(&env, NULL) == -1);
    TEST("validate null frame",     omi_sense_validate_frame(NULL) == -1);

    /* ── Test 8: Frame validation ── */
    printf("\n[Test 8] Frame validation\n");
    ret = omi_sense_validate_frame(&frame);
    TEST("valid frame passes", ret == 0);

    OMI_SenseFrame bad;
    memcpy(&bad, &frame, sizeof(frame));
    bad.preheader[0] = 0x00;
    ret = omi_sense_validate_frame(&bad);
    TEST("bad preheader rejected", ret == -2);

    memcpy(&bad, &frame, sizeof(frame));
    bad.sensor_mask = 0;
    ret = omi_sense_validate_frame(&bad);
    TEST("zero sensor_mask rejected", ret == -3);

    memcpy(&bad, &frame, sizeof(frame));
    bad.geometry_mask = 0;
    ret = omi_sense_validate_frame(&bad);
    TEST("zero geometry_mask rejected", ret == -4);

    /* ── Test 9: pair16 ── */
    printf("\n[Test 9] pair16\n");
    uint32_t p = omi_sense_pair16(0xAAAA, 0xBBBB);
    TEST("pair16 high == first arg", (p >> 16) == 0xAAAA);
    TEST("pair16 low == second arg", (p & 0xFFFF) == 0xBBBB);

    /* ── Test 10: All sensor mask bits distinct ── */
    printf("\n[Test 10] Sensor mask bits\n");
    uint32_t all_sensors = 0;
    all_sensors |= OMI_SENSE_SENSOR_LIGHT;
    all_sensors |= OMI_SENSE_SENSOR_SOUND;
    all_sensors |= OMI_SENSE_SENSOR_TOUCH;
    all_sensors |= OMI_SENSE_SENSOR_GESTURE;
    all_sensors |= OMI_SENSE_SENSOR_MOTION;
    all_sensors |= OMI_SENSE_SENSOR_ORIENTATION;
    all_sensors |= OMI_SENSE_SENSOR_PROXIMITY;
    all_sensors |= OMI_SENSE_SENSOR_CAMERA;
    all_sensors |= OMI_SENSE_SENSOR_ENV;
    all_sensors |= OMI_SENSE_SENSOR_NETWORK;
    all_sensors |= OMI_SENSE_SENSOR_DOM;
    all_sensors |= OMI_SENSE_SENSOR_HARDWARE;
    TEST("12 sensor bits fit in uint32", all_sensors < 0x10000000u);

    /* ── Test 11: All geometry mask bits distinct ── */
    printf("\n[Test 11] Geometry mask bits\n");
    uint32_t all_geoms = 0;
    all_geoms |= OMI_SENSE_GEOM_RING;
    all_geoms |= OMI_SENSE_GEOM_PERSONAL;
    all_geoms |= OMI_SENSE_GEOM_GARDEN;
    all_geoms |= OMI_SENSE_GEOM_RADIAL;
    all_geoms |= OMI_SENSE_GEOM_TETRAHEDRAL;
    all_geoms |= OMI_SENSE_GEOM_5CELL;
    all_geoms |= OMI_SENSE_GEOM_11CELL;
    all_geoms |= OMI_SENSE_GEOM_ORBITAL;
    TEST("8 geometry bits fit in uint32", all_geoms < 0x100u);

    /* ── Test 12: Full multi-sensor frame ── */
    printf("\n[Test 12] Multi-sensor frame\n");
    OMI_SenseFrame msf;
    memset(&msf, 0, sizeof(msf));
    memcpy(msf.preheader, "\xFF\x0C\x1C\x1D\x1E\x1F\x20\xFF", 8);
    msf.cycle         = 1001;
    msf.source_id     = 0x07;
    msf.sensor_mask   = OMI_SENSE_SENSOR_LIGHT | OMI_SENSE_SENSOR_TOUCH
                        | OMI_SENSE_SENSOR_PROXIMITY | OMI_SENSE_SENSOR_ENV;
    msf.geometry_mask = OMI_SENSE_GEOM_GARDEN | OMI_SENSE_GEOM_RADIAL;
    msf.orientation   = 0;
    for (int i = 0; i < 11; i++)
        msf.orientation = omi_sense_set_orientation(msf.orientation, (uint8_t)i, (uint8_t)(i % 3));
    msf.sample_word   = 0xAABBCCDD;
    msf.delta_word    = 0x11223344;
    msf.omi_witness   = omi_sense_omi_witness(0x1234, 0x55AA);
    msf.imo_witness   = omi_sense_imo_witness(0x1234, 0x55AA);
    msf.flags         = OMI_SENSE_EFFECT_HARDWARE;

    ret = omi_sense_validate_frame(&msf);
    TEST("multi-sensor frame validates", ret == 0);

    OMI_512_Envelope me;
    omi_sense_frame_to_env(&msf, &me);
    TEST("multi-sensor envelope has valid pre-header",
         me.gauge[0] == 0xFF && me.gauge[7] == 0xFF);
    TEST("multi-sensor envelope has sense marker",
         me.gauge[1] == OMI_SENSE_MARKER);

    OMI_SenseFrame msb;
    omi_sense_env_to_frame(&me, &msb);
    TEST("multi-sensor round-trip cycle",  msb.cycle == 1001);
    TEST("multi-sensor round-trip source", msb.source_id == 0x07);
    TEST("multi-sensor round-trip sensors", msb.sensor_mask == msf.sensor_mask);
    TEST("multi-sensor round-trip geometry", msb.geometry_mask == msf.geometry_mask);
    TEST("multi-sensor round-trip orientation", msb.orientation == msf.orientation);
    TEST("multi-sensor round-trip flags", msb.flags == OMI_SENSE_EFFECT_HARDWARE);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
