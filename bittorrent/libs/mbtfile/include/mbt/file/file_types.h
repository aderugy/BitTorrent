#ifndef FILE_TYPES_H
#define FILE_TYPES_H

#include <mbt/be/types_mbtbe.h>
#include <mbt/file/file_handler.h>

#define MBT_H_LENGTH 20

#define MBT_PIECE_SIZE (1 << 18)
#define MBT_PIECE_NB_BLOCK (MBT_PIECE_SIZE / MBT_BLOCK_SIZE)

struct mbt_file_handler
{
    struct mbt_str *h;
    struct mbt_str *name;

    size_t len_piece;

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

#endif // !FILE_TYPES_H
