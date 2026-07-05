#ifndef OMI_ENV_H
#define OMI_ENV_H

#include <stdint.h>
#include <stddef.h>

#define OMI_ENV_SIZE       64
#define OMI_PREHEADER_SIZE 8

#define OMI_BB_GAUGE_MASK       0x000000000000007FULL
#define OMI_BB_DPLUS_SHIFT      7
#define OMI_BB_DPLUS_MASK       0x0000000000001F80ULL
#define OMI_BB_DMINUS_SHIFT     12
#define OMI_BB_DMINUS_MASK      0x000000000001E000ULL
#define OMI_BB_BRIDGE_1E_BIT    17
#define OMI_BB_BRIDGE_78_BIT    18
#define OMI_BB_SEAL_7F_BIT      19
#define OMI_BB_BOOT_7C00_BIT    20
#define OMI_BB_BRIDGE_AA55_BIT  21
#define OMI_BB_PLACE_SHIFT      22
#define OMI_BB_PLACE_MASK       0x00000000FFC00000ULL
#define OMI_BB_RESULT_SHIFT     32
#define OMI_BB_RESULT_MASK      0x0000FFFF00000000ULL
#define OMI_BB_FOLD_SHIFT       48
#define OMI_BB_FOLD_MASK        0xFFFF000000000000ULL

typedef struct {
    uint8_t gauge[8];
    uint8_t orientation[8];
    uint8_t recovery[8];
    uint8_t target[8];
    uint8_t relation[32];
} OMI_512_Envelope;

typedef enum {
    GAUGE_CLASS_CONTROL,
    GAUGE_CLASS_PRINTABLE,
    GAUGE_CLASS_DEL
} GaugeClass;

typedef enum {
    DIAG_NONE,
    DIAG_PLUS,
    DIAG_MINUS,
    DIAG_BALANCED
} DiagClass;

typedef enum {
    GAUGE_ACTION_NONE,
    GAUGE_ACTION_PLACE,
    GAUGE_ACTION_REGISTER_INJECT,
    GAUGE_ACTION_KERNEL_READ,
    GAUGE_ACTION_RECORD_CLOSE,
    GAUGE_ACTION_SYSTEM_WITNESS,
    GAUGE_ACTION_SEAL,
    GAUGE_ACTION_BOOT_PAGE,
    GAUGE_ACTION_EXTERNAL_BRIDGE
} GaugeAction;

typedef struct {
    uint8_t code;
    GaugeClass cls;
    DiagClass diag;
    GaugeAction action;
    uint16_t s[8];
    uint32_t payload;
    uint32_t mask;
    uint32_t car;
    uint32_t cdr;
    uint32_t bridge;
    uint8_t active;
    uint32_t car_addr;
    uint32_t cdr_addr;
    uint8_t eval_depth;
    uint8_t is_lambda;
} OmiGaugeEntry;

int omi_env_parse(const uint8_t* data, size_t len, OMI_512_Envelope* env);
int omi_env_validate(const OMI_512_Envelope* env);
int omi_env_to_bitboard(const OMI_512_Envelope* env, uint64_t* board);
int omi_env_to_relation(const OMI_512_Envelope* env, char* out, size_t out_len);

uint64_t omi_bb_apply(uint64_t board, uint8_t byte, uint8_t place);
uint64_t omi_bb_fold(uint64_t board);
uint64_t omi_bb_set_gauge(uint64_t board, uint8_t code);
uint64_t omi_bb_set_flag(uint64_t board, unsigned bit);
int      omi_bb_get_flag(uint64_t board, unsigned bit);
uint8_t  omi_bb_get_gauge(uint64_t board);
uint8_t  omi_bb_get_place(uint64_t board);
uint32_t omi_bb_get_result(uint64_t board);

const OmiGaugeEntry* omi_gauge_lookup(uint8_t code);
void omi_gauge_init(void);

uint8_t hex_to_nibble(char c);
char nibble_to_hex(uint8_t n);

#endif
