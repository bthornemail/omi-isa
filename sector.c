#include "sector.h"
#include <string.h>

int sector_iter_init(SectorIterator* iter, const uint8_t* data, size_t len) {
    if (!iter || !data) return -1;
    if (len < OMI_SECTOR_SIZE) return -2;
    memset(iter, 0, sizeof(SectorIterator));
    iter->data = data;
    iter->len = (len < OMI_SECTOR_SIZE) ? len : OMI_SECTOR_SIZE;
    iter->cell_index = -1;
    for (int i = 0; i < OMI_CELLS_PER_SECTOR; i++) {
        const uint8_t* cell_data = data + (i * OMI_ENV_SIZE);
        if ((size_t)(i * OMI_ENV_SIZE + OMI_ENV_SIZE) <= len) {
            int r = omi_env_parse(cell_data, OMI_ENV_SIZE, &iter->cells[i]);
            if (r == 0) {
                iter->cell_valid[i] = (omi_env_validate(&iter->cells[i]) == 0);
                if (iter->cell_valid[i])
                    omi_env_to_bitboard(&iter->cells[i], &iter->cell_bitboard[i]);
            } else {
                iter->cell_valid[i] = 0;
            }
        } else {
            iter->cell_valid[i] = 0;
        }
    }
    return 0;
}

int sector_iter_next(SectorIterator* iter, OMI_512_Envelope* env, uint64_t* board) {
    if (!iter) return -1;
    iter->cell_index++;
    while (iter->cell_index < OMI_CELLS_PER_SECTOR) {
        if (iter->cell_valid[iter->cell_index]) {
            if (env) *env = iter->cells[iter->cell_index];
            if (board) *board = iter->cell_bitboard[iter->cell_index];
            return 0;
        }
        iter->cell_index++;
    }
    return -2;
}

int sector_iter_count(const SectorIterator* iter) {
    if (!iter) return 0;
    int count = 0;
    for (int i = 0; i < OMI_CELLS_PER_SECTOR; i++)
        if (iter->cell_valid[i]) count++;
    return count;
}
