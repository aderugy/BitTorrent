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

struct mbt_bite
{
    char *path;
    size_t len;
    size_t offset;
    struct mbt_bite *next;
};

bool mbt_piece_block_is_received(struct mbt_file_handler *fh,
                                 size_t piece_index, size_t block_index)
{
    return fh->pieces[piece_index]->status[block_index]
        == BLOCK_STATUS_COMPLETED;
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

    for (size_t i = 0; i < piece->nb_blocks; i++)
    {
        if (piece->status[i] != BLOCK_STATUS_COMPLETED)
        {
            return MBT_PIECE_DOWNLOADING;
        }
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
    char *copy = calloc(1000, sizeof(char));
    for (size_t i = 0; i < path_length - 1; i++)
    {
        logger("path: %s\n", path[i]->data);
        strcat(copy, path[i]->data);
        strcat(copy, "/");
        mkdir(copy, 0777);
    }

    strcat(copy, path[path_length - 1]->data);

    return copy;
}

static struct mbt_bite *mbt_piece_get_files(struct mbt_file_handler *fh,
                                            uint32_t piece_index)
{
    struct mbt_bite *mb = NULL;
    size_t offset = piece_index * MBT_PIECE_SIZE;

    size_t i = 0;
    for (; offset > 0 && fh->files_info[i]; i++)
    {
        struct mbt_files_info *fi = fh->files_info[i];

        if (fi->size > offset)
        {
            break;
        }
        offset -= fi->size;
    }

    struct mbt_bite *next = mb;
    size_t piece_len = fh->pieces[piece_index]->size;
    for (; fh->files_info[i] && piece_len > 0; i++)
    {
        struct mbt_files_info *fi = fh->files_info[i];
        struct mbt_bite *bite = xcalloc(1, sizeof(struct mbt_bite));
        bite->offset = offset;
        bite->len = fi->size - offset;
        bite->path = create_path(fi->path, fi->path_length);
        offset = 0;

        if (bite->len > piece_len)
        {
            bite->len = piece_len;
        }
        piece_len -= bite->len;

        if (!next)
        {
            mb = bite;
            next = bite;
        }
        else
        {
            next->next = bite;
            next = bite;
        }
    }

    return mb;
}

static bool write_in_file(struct mbt_bite *bite, size_t piece_index,
                          size_t written, const char *data)
{
    if (!bite)
    {
        return true;
    }

    FILE *f = fopen(bite->path, "w");
    if (!f)
    {
        errx(EXIT_FAILURE, "write_in_file: fopen");
    }

    if (fseek(f, bite->offset, SEEK_SET))
    {
        perror(NULL);
        errx(EXIT_FAILURE, "write_in_file: fseek");
    }

    size_t bytes = 0;
    while (bytes < bite->len)
    {
        int w = fwrite(data + written, sizeof(char), bite->len - bytes, f);
        if (w <= 0)
        {
            errx(EXIT_FAILURE, "write_in_file: fwrite");
        }

        bytes += w;
    }

    fclose(f);
    struct mbt_bite *next = bite->next;

    free(bite->path);
    free(bite);

    return write_in_file(next, piece_index, written + bytes, data);
}

bool mbt_piece_write(struct mbt_file_handler *fh, size_t piece_index)
{
    if (fh->nb_pieces <= piece_index)
    {
        return false;
    }

    void *v_buf = fh->pieces[piece_index]->data;

    struct mbt_bite *bite = mbt_piece_get_files(fh, piece_index);

    return write_in_file(bite, piece_index, 0, v_buf);
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
