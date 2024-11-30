#include <err.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/hash.h>
#include <mbt/utils/logger.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mbt/file/file_handler.h"
#include "mbt/file/piece.h"
#include "stdio.h"

bool mbt_piece_block_is_received(struct mbt_file_handler *fh,
                                 size_t piece_index, size_t block_index)
{
    return fh->pieces[piece_index]->status[block_index];
}

void mbt_piece_block_set_received(struct mbt_file_handler *fh,
                                  size_t piece_index, size_t block_index,
                                  bool received)
{
    fh->pieces[piece_index]->status[block_index] = received;
}

size_t mbt_piece_get_nb_blocks(struct mbt_file_handler *fh, size_t piece_index)
{
    if (piece_index >= fh->nb_pieces)
    {
        errx(EXIT_FAILURE, "mbt_piece_get_nb_blocks: index out of bounds");
    }

    struct mbt_piece *piece = fh->pieces[piece_index];

    return piece->size / MBT_BLOCK_SIZE
        + (piece->size % MBT_BLOCK_SIZE > 0 ? 1 : 0);
}

enum mbt_piece_status mbt_piece_check(struct mbt_file_handler *fh,
                                      size_t piece_index)
{
    if (piece_index >= fh->nb_pieces)
    {
        errx(EXIT_FAILURE, "mbt_piece_check: index out of bounds");
    }

    struct mbt_piece *piece = fh->pieces[piece_index];

    // Check if all blocks are dl
    if (!piece->data)
    {
        return MBT_PIECE_DOWNLOADING;
    }

    // Compare hashes
    void *v_data = piece->data;
    char *cur_hash = sha1(v_data, piece->size);

    int res = memcmp(cur_hash, piece->h, MBT_H_LENGTH) == 0;
    free(cur_hash);

    return res ? MBT_PIECE_VALID : MBT_PIECE_INVALID;
}

char *create_path(struct mbt_str **path, size_t path_length)
{
    char *copy = calloc(64, sizeof(char));
    for (size_t i = 0; i < path_length - 1; i++)
    {
        logger("path: %s\n", path[i]->data);
        strcat(copy, path[i]->data);
        strcat(copy, "/");
        mkdir(copy, 0777);
    }

    strcat(copy, path[path_length - 1]->data);

    strcat(copy, "t");
    return copy;
}

bool write_in_file(const char *path, const char *start_data,
                   const char *end_data)
{
    FILE *file = fopen(path, "w");

    if (!file)
    {
        return false;
    }

    for (const char *i = start_data; i < end_data; i++)
    {
        fputc(*i, file);
    }
    fclose(file);

    return true;
}

bool mbt_piece_write(struct mbt_file_handler *fh, size_t piece_index)
{
    logger("mbt_piece");
    if (fh->nb_pieces <= piece_index)
    {
        return false;
    }
    size_t read_piece_size = 0;
    logger("piece_index: %zu\n", piece_index);
    for (size_t i = 0; fh->files_info[i]; i++)
    {
        logger("i: %zu\n", i);
        char *path = create_path(fh->files_info[i]->path,
                                 fh->files_info[i]->path_length);
        logger("path: %s\n", path);
        if (!path)
        {
            free(path);
            return false;
        }

        const char *start_data =
            mbt_piece_get_data(fh, piece_index) + read_piece_size;
        const char *end_data = start_data + fh->files_info[i]->size - 1;
        read_piece_size += fh->files_info[i]->size - 1;

        if (!write_in_file(path, start_data, end_data))
        {
            free(path);
            return false;
        }
        free(path);
    }
    return true;
}

bool mbt_piece_write_block(struct mbt_file_handler *fh, struct mbt_str *data,
                           uint32_t piece_index, uint32_t piece_offset)
{
    if (piece_index >= fh->nb_pieces)
    {
        return false;
    }

    if (data->size < MBT_BLOCK_SIZE && piece_index < fh->nb_pieces - 1)
    {
        return false;
    }

    size_t block_index = piece_offset / MBT_BLOCK_SIZE;
    struct mbt_piece *piece = fh->pieces[piece_index];
    if (!piece->data)
    {
        piece->data = xcalloc(MBT_PIECE_NB_BLOCK, MBT_BLOCK_SIZE);
    }

    struct mbt_block *block = piece->data + block_index;
    memcpy(block, data->data, data->size);

    mbt_piece_block_set_received(fh, piece_index, block_index, true);
    return true;
}
