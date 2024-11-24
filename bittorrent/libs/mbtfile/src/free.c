#include <mbt/be/torrent.h>
#include <mbt/file/file_types.h>
#include <mbt/file/piece.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

void mbt_file_handler_free(struct mbt_file_handler *fh)
{
    if (!fh)
    {
        return;
    }

    for (size_t i = 0; fh->pieces[i]; i++)
    {
        struct mbt_piece *piece = fh->pieces[i];
        mbt_piece_dtor(piece);
        free(piece);
    }
    free(fh->pieces);

    mbt_str_free(fh->h);
    mbt_str_free(fh->name);

    free(fh);
}
