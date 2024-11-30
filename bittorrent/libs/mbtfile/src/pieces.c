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

    memset(p->status, 0, MBT_PIECE_NB_BLOCK * sizeof(int));
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
    memcpy((piece->data), data, size);
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
