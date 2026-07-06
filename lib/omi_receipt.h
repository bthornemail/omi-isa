#ifndef OMI_RECEIPT_H
#define OMI_RECEIPT_H

#include "omienv.h"

#define OMI_RECEIPT_PATH_MAX  16
#define OMI_RECEIPT_LAYERS    5

typedef struct {
    uint32_t layer_values[OMI_RECEIPT_LAYERS];
    uint8_t  gauge_action;
    uint8_t  path[OMI_RECEIPT_PATH_MAX];
    int      path_len;
    int      valid;
} OMI_Receipt;

int  omi_receipt_compute(const OMI_512_Envelope* env, OMI_Receipt* receipt);
int  omi_receipt_verify(const OMI_512_Envelope* env, const OMI_Receipt* receipt);
int  omi_receipt_verify_meta64(const OMI_512_Envelope* env);
int  omi_receipt_to_string(const OMI_Receipt* receipt, char* out, size_t out_len);

#endif
