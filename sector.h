#ifndef OMI_SECTOR_H
#define OMI_SECTOR_H

#include "omienv.h"

#define OMI_SECTOR_SIZE       512
#define OMI_CELLS_PER_SECTOR  8

typedef struct {
    const uint8_t* data;
    size_t len;
    int cell_index;
    OMI_512_Envelope cells[OMI_CELLS_PER_SECTOR];
    int cell_valid[OMI_CELLS_PER_SECTOR];
    uint64_t cell_bitboard[OMI_CELLS_PER_SECTOR];
} SectorIterator;

int sector_iter_init(SectorIterator* iter, const uint8_t* data, size_t len);
int sector_iter_next(SectorIterator* iter, OMI_512_Envelope* env, uint64_t* board);
int sector_iter_count(const SectorIterator* iter);

#endif
