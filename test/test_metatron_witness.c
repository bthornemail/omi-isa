#include <stdint.h>
#include <stdio.h>

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

static uint16_t bounded16(uint16_t value)
{
    return (uint16_t)(value & UINT16_C(0xFFFF));
}

static uint16_t xor16(uint16_t left, uint16_t right)
{
    return bounded16((uint16_t)(left ^ right));
}

static unsigned popcount16(uint16_t value)
{
    unsigned count = 0;
    for (unsigned bit = 0; bit < 16; bit++) {
        count += (unsigned)((value >> bit) & UINT16_C(1));
    }
    return count;
}

static unsigned zerocount16(uint16_t value)
{
    return 16u - popcount16(value);
}

static int balanced16(uint16_t value)
{
    return popcount16(value) == 8u && zerocount16(value) == 8u;
}

static int metatron_is_validation_authority(void)
{
    return 0;
}

static int metatron_may_project_after_validation(int validated)
{
    return validated ? 1 : 0;
}

int main(void)
{
    const uint16_t witness = UINT16_C(0xAA55);
    const uint16_t inverse = UINT16_C(0x55AA);
    const uint16_t full = UINT16_C(0xFFFF);

    printf("=== Metatron Witness OMI-ISA Fixture Tests ===\n\n");

    test("0xAA55 is the Metatron alternating witness register",
         witness == UINT16_C(0xAA55));
    test("0x55AA is the inverse alternating witness register",
         inverse == UINT16_C(0x55AA));
    test("0xAA55 has exactly 8 one bits",
         popcount16(witness) == 8u);
    test("0xAA55 has exactly 8 zero bits",
         zerocount16(witness) == 8u);
    test("0x55AA has exactly 8 one bits",
         popcount16(inverse) == 8u);
    test("0x55AA has exactly 8 zero bits",
         zerocount16(inverse) == 8u);
    test("0xAA55 is balanced",
         balanced16(witness));
    test("0x55AA is balanced",
         balanced16(inverse));
    test("0xAA55 ^ 0x55AA == 0xFFFF",
         xor16(witness, inverse) == full);
    test("0xAA55 ^ 0xFFFF == 0x55AA",
         xor16(witness, full) == inverse);
    test("0x55AA ^ 0xFFFF == 0xAA55",
         xor16(inverse, full) == witness);
    test("0xAA55 is not validation authority",
         !metatron_is_validation_authority());
    test("Metatron projects after validation",
         metatron_may_project_after_validation(1));
    test("Metatron rejects projection before validation",
         !metatron_may_project_after_validation(0));

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
