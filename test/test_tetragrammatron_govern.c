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

static uint16_t nibble_sum(uint8_t start, uint8_t step, uint8_t count)
{
    uint16_t sum = 0;
    for (uint8_t index = 0; index < count; index++) {
        sum = bounded16((uint16_t)(sum + (uint16_t)(start + (uint8_t)(step * index))));
    }
    return sum;
}

static uint16_t full_nibble_field_sum(void)
{
    uint16_t sum = 0;
    for (uint8_t value = 0; value <= UINT8_C(0x0F); value++) {
        sum = bounded16((uint16_t)(sum + value));
    }
    return sum;
}

static uint16_t fold_carry_left1(uint16_t value)
{
    return bounded16((uint16_t)(value << 1));
}

static int is_place_controller(uint8_t value)
{
    return value <= UINT8_C(0x1F);
}

static int is_register_injection(uint8_t value)
{
    return value >= UINT8_C(0x20) && value <= UINT8_C(0x2F);
}

static int is_kernel_reader(uint8_t value)
{
    return value >= UINT8_C(0x30) && value <= UINT8_C(0x3F);
}

static int is_byte_gauge_value(uint16_t value)
{
    return value <= UINT16_C(0x007F);
}

int main(void)
{
    const uint16_t diagonal_closure = UINT16_C(0x001E);
    const uint16_t full_system_witness = UINT16_C(0x0078);
    const uint16_t declaration_opener = UINT16_C(0x003C);
    const uint16_t local_ascii_seal = UINT16_C(0x007F);
    const uint16_t boot_page_witness = UINT16_C(0x7C00);
    const uint16_t external_bridge_word = UINT16_C(0xAA55);

    printf("=== Tetragrammatron Govern OMI-ISA Fixture Tests ===\n\n");

    test("D+ diagonal {0,5,A,F} closes to 0x1E",
         nibble_sum(UINT8_C(0x00), UINT8_C(0x05), UINT8_C(4)) == diagonal_closure);
    test("D- diagonal {3,6,9,C} closes to 0x1E",
         nibble_sum(UINT8_C(0x03), UINT8_C(0x03), UINT8_C(4)) == diagonal_closure);
    test("0x1E is place-controller record separator",
         is_place_controller(UINT8_C(0x1E)));
    test("full nibble field closes to 0x78",
         full_nibble_field_sum() == full_system_witness);
    test("0x3C is in the kernel reader band",
         is_kernel_reader(UINT8_C(0x3C)));
    test("0x3C is declaration opener",
         declaration_opener == UINT16_C(0x003C));
    test("0x3C << 1 == 0x78",
         fold_carry_left1(declaration_opener) == full_system_witness);
    test("0x78 is the full hexadecimal system witness",
         full_system_witness == UINT16_C(0x0078));
    test("0x7F is the local ASCII seal",
         local_ascii_seal == UINT16_C(0x007F));
    test("0x7C00 is the boot page witness",
         boot_page_witness == UINT16_C(0x7C00));
    test("0x20 is register injection band",
         is_register_injection(UINT8_C(0x20)));
    test("0x2E is register injection dot",
         is_register_injection(UINT8_C(0x2E)));
    test("0x30 is kernel reader band",
         is_kernel_reader(UINT8_C(0x30)));
    test("0x3F is kernel reader band",
         is_kernel_reader(UINT8_C(0x3F)));
    test("0x7F remains inside byte gauge",
         is_byte_gauge_value(local_ascii_seal));
    test("0xAA55 is not a byte gauge value",
         !is_byte_gauge_value(external_bridge_word));

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
