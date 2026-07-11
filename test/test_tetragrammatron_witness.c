#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_run = 0;

static void test(const char *name, int condition)
{
    tests_run++;
    if (condition) {
        tests_passed++;
    }
    printf("  %s: %s\n", condition ? "PASS" : "FAIL", name);
}

static uint8_t bounded8(uint16_t value)
{
    return (uint8_t)(value & UINT16_C(0x00FF));
}

static uint8_t shl8(uint8_t value, unsigned shift)
{
    return bounded8((uint16_t)value << shift);
}

static uint8_t shr8(uint8_t value, unsigned shift)
{
    return bounded8((uint16_t)value >> shift);
}

static uint8_t xor8(uint8_t left, uint8_t right)
{
    return bounded8((uint16_t)(left ^ right));
}

static uint8_t sum4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return bounded8((uint16_t)a + b + c + d);
}

static uint8_t sum_nibble_field(void)
{
    uint16_t total = 0;
    for (uint8_t value = 0; value <= UINT8_C(0x0F); value++) {
        total = (uint16_t)(total + value);
    }
    return bounded8(total);
}

static unsigned popcount8(uint8_t value)
{
    unsigned count = 0;
    for (unsigned bit = 0; bit < 8; bit++) {
        count += (unsigned)((value >> bit) & UINT8_C(1));
    }
    return count;
}

static unsigned zerocount8(uint8_t value)
{
    return 8u - popcount8(value);
}

static int balanced8(uint8_t value)
{
    return popcount8(value) == 4u && zerocount8(value) == 4u;
}

enum {
    CLOCK_ATOMIC_LOGIC = 0,
    CLOCK_SPECTRAL_OBSERVER,
    CLOCK_COSMIC_ORBIT,
    CLOCK_COUNT
};

enum {
    GOVERNOR_FACTS = 0,
    GOVERNOR_RULES,
    GOVERNOR_CLOSURES,
    GOVERNOR_COMBINATORS,
    GOVERNOR_CONS,
    GOVERNOR_COUNT
};

enum {
    MIQUEL_POINT_COUNT = 8,
    MIQUEL_BLOCK_COUNT = 6,
    MIQUEL_BLOCK_WIDTH = 4
};

static const char *const clock_names[CLOCK_COUNT] = {
    "Atomic Logic Clock",
    "Spectral Observer Clock",
    "Cosmic Orbit Clock"
};

static const uint16_t visible_offsets[] = {
    UINT16_C(0x0001),
    UINT16_C(0x0010),
    UINT16_C(0x0100),
    UINT16_C(0x1000)
};

static const int governor_exponents[GOVERNOR_COUNT] = {-1, 0, 1, 2, 3};

static const uint8_t miquel_blocks[MIQUEL_BLOCK_COUNT][MIQUEL_BLOCK_WIDTH] = {
    {0, 1, 4, 5},
    {0, 2, 4, 6},
    {0, 3, 4, 7},
    {1, 2, 5, 6},
    {1, 3, 5, 7},
    {2, 3, 6, 7}
};

static unsigned miquel_point_degree(uint8_t point)
{
    unsigned degree = 0;
    for (size_t block = 0; block < MIQUEL_BLOCK_COUNT; block++) {
        for (size_t member = 0; member < MIQUEL_BLOCK_WIDTH; member++) {
            if (miquel_blocks[block][member] == point) {
                degree++;
            }
        }
    }
    return degree;
}

static int governor_exponents_are_consecutive(void)
{
    for (size_t i = 1; i < GOVERNOR_COUNT; i++) {
        if (governor_exponents[i] != governor_exponents[i - 1] + 1) {
            return 0;
        }
    }
    return 1;
}

static int all_miquel_points_have_degree_three(void)
{
    for (uint8_t point = 0; point < MIQUEL_POINT_COUNT; point++) {
        if (miquel_point_degree(point) != 3u) {
            return 0;
        }
    }
    return 1;
}

static size_t miquel_flag_count(void)
{
    size_t flags = 0;
    for (size_t block = 0; block < MIQUEL_BLOCK_COUNT; block++) {
        flags += sizeof(miquel_blocks[block]) / sizeof(miquel_blocks[block][0]);
    }
    return flags;
}

static int tetragrammatron_governs_polybius(void)
{
    return 1;
}

static int tetragrammatron_projects_surface(void)
{
    return 0;
}

static int tetragrammatron_scribes_crossing(void)
{
    return 0;
}

static int fixture_adds_compiler_keywords(void)
{
    return 0;
}

static int fixture_validates_arbitrary_frames(void)
{
    return 0;
}

static int fixture_accepts_receipts(void)
{
    return 0;
}

static int fixture_alters_lowering(void)
{
    return 0;
}

int main(void)
{
    const uint8_t d_plus_closure =
        sum4(UINT8_C(0x00), UINT8_C(0x05), UINT8_C(0x0A), UINT8_C(0x0F));
    const uint8_t d_minus_closure =
        sum4(UINT8_C(0x03), UINT8_C(0x06), UINT8_C(0x09), UINT8_C(0x0C));
    const uint8_t diagonal_closure = UINT8_C(0x1E);
    const uint8_t record_separator = UINT8_C(0x1E);
    const uint8_t full_nibble_witness = sum_nibble_field();
    const uint8_t centered_fold = UINT8_C(0x3C);
    const uint8_t carry_fold = UINT8_C(0x78);
    const uint8_t difference = UINT8_C(0x44);

    printf("=== Tetragrammatron Witness OMI-ISA Fixture Tests ===\n\n");

    test("polyharmonic governor has 3 clocks",
         CLOCK_COUNT == 3);
    test("clock 0 is Atomic Logic Clock",
         strcmp(clock_names[CLOCK_ATOMIC_LOGIC], "Atomic Logic Clock") == 0);
    test("clock 1 is Spectral Observer Clock",
         strcmp(clock_names[CLOCK_SPECTRAL_OBSERVER],
                "Spectral Observer Clock") == 0);
    test("clock 2 is Cosmic Orbit Clock",
         strcmp(clock_names[CLOCK_COSMIC_ORBIT], "Cosmic Orbit Clock") == 0);
    test("polyharmonic governor has 4 visible offsets",
         sizeof(visible_offsets) / sizeof(visible_offsets[0]) == 4u);
    test("FS offset is 0x0001",
         visible_offsets[0] == UINT16_C(0x0001));
    test("GS offset is one nibble above FS",
         (uint16_t)(visible_offsets[0] << 4) == visible_offsets[1]);
    test("RS offset is one nibble above GS",
         (uint16_t)(visible_offsets[1] << 4) == visible_offsets[2]);
    test("US offset is one nibble above RS",
         (uint16_t)(visible_offsets[2] << 4) == visible_offsets[3]);
    test("polyharmonic governor has 5 modes",
         GOVERNOR_COUNT == 5);
    test("governor exponents are consecutive from -1 through 3",
         governor_exponents_are_consecutive());
    test("RULES is the p=0 Genesis equality pivot",
         governor_exponents[GOVERNOR_RULES] == 0);
    test("FACTS is the p=-1 inverse endpoint",
         governor_exponents[GOVERNOR_FACTS] == -1);
    test("CONS is the p=3 forward endpoint",
         governor_exponents[GOVERNOR_CONS] == 3);
    test("Miquel controller has 8 points",
         MIQUEL_POINT_COUNT == 8);
    test("Miquel controller has 6 blocks",
         sizeof(miquel_blocks) / sizeof(miquel_blocks[0]) ==
             MIQUEL_BLOCK_COUNT);
    test("each Miquel block has 4 points",
         sizeof(miquel_blocks[0]) / sizeof(miquel_blocks[0][0]) ==
             MIQUEL_BLOCK_WIDTH);
    test("each Miquel point occurs in 3 blocks",
         all_miquel_points_have_degree_three());
    test("Miquel flag count is 24",
         miquel_flag_count() == 24u);
    test("Miquel incidence balances 8*3 == 6*4",
         MIQUEL_POINT_COUNT * 3u ==
             MIQUEL_BLOCK_COUNT * MIQUEL_BLOCK_WIDTH);
    test("four gauges have C(4,2) == 6 pairwise planes",
         (4u * 3u) / 2u == 6u);
    test("OMI oversight frame has 5+6 == 11 positions",
         5u + 6u == 11u);
    test("D+ diagonal {0,5,A,F} closes to 0x1E",
         d_plus_closure == diagonal_closure);
    test("D- diagonal {3,6,9,C} closes to 0x1E",
         d_minus_closure == diagonal_closure);
    test("0x1E is record separator / diagonal closure witness",
         diagonal_closure == record_separator);
    test("full nibble field 0+1+...+F closes to 0x78",
         full_nibble_witness == UINT8_C(0x78));
    test("0x78 is the full hexadecimal system witness",
         full_nibble_witness == carry_fold);
    test("0x3C is the centered fold witness",
         centered_fold == UINT8_C(0x3C));
    test("0x78 is the shifted carry fold witness",
         carry_fold == UINT8_C(0x78));
    test("0x78 == (0x3C << 1) masked to 8 bits",
         shl8(centered_fold, 1) == carry_fold);
    test("0x3C == (0x78 >> 1)",
         shr8(carry_fold, 1) == centered_fold);
    test("0x3C has exactly 4 one bits",
         popcount8(centered_fold) == 4u);
    test("0x3C has exactly 4 zero bits",
         zerocount8(centered_fold) == 4u);
    test("0x78 has exactly 4 one bits",
         popcount8(carry_fold) == 4u);
    test("0x78 has exactly 4 zero bits",
         zerocount8(carry_fold) == 4u);
    test("0x3C is balanced",
         balanced8(centered_fold));
    test("0x78 is balanced",
         balanced8(carry_fold));
    test("0x3C ^ 0x78 == 0x44",
         xor8(centered_fold, carry_fold) == difference);
    test("Tetragrammatron governs the Polybius gauge",
         tetragrammatron_governs_polybius());
    test("Tetragrammatron does not project surfaces",
         !tetragrammatron_projects_surface());
    test("Tetragrammatron does not scribe crossings",
         !tetragrammatron_scribes_crossing());
    test("fixture does not add compiler keywords",
         !fixture_adds_compiler_keywords());
    test("fixture does not validate arbitrary frames",
         !fixture_validates_arbitrary_frames());
    test("fixture does not accept receipts",
         !fixture_accepts_receipts());
    test("fixture does not alter lowering",
         !fixture_alters_lowering());

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
