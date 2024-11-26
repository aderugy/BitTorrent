#include <err.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/hash.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <string.h>

#include "ctype.h"
#include "mbt/file/file_handler.h"
#include "mbt/file/piece.h"

struct mbt_piece *mbt_piece_init(char *h)
{
    struct mbt_piece *piece = xcalloc(1, sizeof(struct mbt_piece));

    piece->h = xcalloc(MBT_H_LENGTH + 1, sizeof(char));
    memcpy(piece->h, h, MBT_H_LENGTH);

    return piece;
}

size_t mbt_file_handler_get_nb_pieces(struct mbt_file_handler *fh)
{
    return fh->nb_pieces;
}

const char *mbt_file_handler_get_pieces_hash(struct mbt_file_handler *fh)
{
    return MBT_CVIEW_OF(*fh->h).data;
}

void mbt_piece_dtor(struct mbt_piece *p)
{
    if (!p)
    {
        return;
    }

    if (p->data)
    {
        free(p->data);
        p->data = NULL;
    }

    if (p->h)
    {
        free(p->h);
        p->h = NULL;
    }

    memset(p->status, 0, MBT_PIECE_NB_BLOCK * sizeof(bool));
}

const char *mbt_piece_get_data(struct mbt_file_handler *fh, size_t piece_index)
{
    if (piece_index >= fh->nb_pieces)
    {
        errx(EXIT_FAILURE, "mbt_piece_get_data: index out of bounds");
    }

    if (!fh->pieces[piece_index])
    {
        errx(EXIT_FAILURE, "mbt_piece_get_data: data is NULL");
    }

    void *data = fh->pieces[piece_index]->data;
    return data;
}

void mbt_piece_set_data(struct mbt_file_handler *fh, size_t piece_index,
                        const char *data, size_t size)
{
    if (piece_index >= fh->nb_pieces || size > MBT_PIECE_SIZE)
    {
        errx(EXIT_FAILURE, "mbt_piece_get_data: index out of bounds");
    }

    struct mbt_piece *piece = fh->pieces[piece_index];
    memcpy(&(piece->data), data, size);
}

bool mbt_compare_hashes(struct mbt_file_handler *fh, size_t piece_index,
                        const char *received_data)
{
    if (piece_index >= fh->nb_pieces)
    {
        errx(EXIT_FAILURE, "mbt_compare_hashes: index out of bounds");
    }

    struct mbt_piece *ref = fh->pieces[piece_index];
    char *rec_h = sha1(received_data, strlen(received_data));

    int res = memcmp(rec_h, ref->h, MBT_H_LENGTH) == 0;
    free(rec_h);

    return res;
}

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

bool mbt_piece_write(struct mbt_file_handler *fh, size_t piece_index)
{
    if (fh->nb_pieces <= piece_index)
    {
        return false;
    }

    struct mbt_piece *piece = fh->pieces[piece_index];

    void *vbuf = piece->data;
    char *c = vbuf;
    printf("PIECE WRITTEN:\n");
    for (size_t i = 0; i < piece->size; i++)
    {
        if (isprint(c[i]))
        {
            printf("%c", c[i]);
        }
    }
    printf("\n\n");

    free(piece->data);
    piece->data = NULL;

    piece->completed = true;
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
