#ifndef FILE_TYPES_H
#define FILE_TYPES_H

#include <mbt/be/types_mbtbe.h>
#include <mbt/file/file_handler.h>

#include "bits/stdint-uintn.h"
#include "mbt/utils/utils.h"

#define MBT_H_LENGTH 20

#define MBT_PIECE_SIZE (1 << 18)
#define MBT_PIECE_NB_BLOCK (MBT_PIECE_SIZE / MBT_BLOCK_SIZE)

struct mbt_file_handler
{
    struct mbt_str *h;
    struct mbt_str *name;

    size_t nb_pieces;
    struct mbt_piece **pieces;
};

struct mbt_block
{
    char data[MBT_BLOCK_SIZE];
};

struct mbt_piece
{
    char *h;

    bool status[MBT_PIECE_NB_BLOCK];
    struct mbt_block *data;
};

struct mbt_piece *mbt_piece_init(char *h);
bool mbt_piece_write_block(struct mbt_file_handler *fh, struct mbt_str *data,
                           uint32_t piece_index, uint32_t piece_offset)
    MBT_NONNULL(1, 2);

#endif // !FILE_TYPES_H
