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

static uint16_t dot_xor(uint16_t car, uint16_t cdr)
{
    return bounded16((uint16_t)(car ^ cdr));
}

static uint16_t null_seed(void)
{
    return dot_xor(UINT16_C(0x0000), UINT16_C(0x0000));
}

static uint16_t full_witness_closure(void)
{
    return bounded16((uint16_t)(
        UINT16_C(0x0020) ^
        UINT16_C(0x005F) ^
        UINT16_C(0x0080) ^
        UINT16_C(0x00FF)));
}

int main(void)
{
    printf("=== NULL Ring OMI-ISA Fixture Tests ===\n\n");

    test("(NULL . NULL) folds to null seed", null_seed() == UINT16_C(0x0000));
    test("0x00 ^ 0x20 == 0x20",
         dot_xor(UINT16_C(0x0000), UINT16_C(0x0020)) == UINT16_C(0x0020));
    test("0x20 ^ 0x7F == 0x5F",
         dot_xor(UINT16_C(0x0020), UINT16_C(0x007F)) == UINT16_C(0x005F));
    test("0x7F ^ 0xFF == 0x80",
         dot_xor(UINT16_C(0x007F), UINT16_C(0x00FF)) == UINT16_C(0x0080));
    test("0xFF ^ 0x00 == 0xFF",
         dot_xor(UINT16_C(0x00FF), UINT16_C(0x0000)) == UINT16_C(0x00FF));
    test("full witness closure returns 0x00",
         full_witness_closure() == UINT16_C(0x0000));
    test("execution stays bounded to 16 bits",
         dot_xor(UINT16_C(0xFFFF), UINT16_C(0x0001)) == UINT16_C(0xFFFE));

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
