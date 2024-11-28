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

    for (size_t i = 0; i < fh->nb_pieces; i++)
    {
        struct mbt_piece *piece = fh->pieces[i];
        mbt_piece_dtor(piece);
        free(piece);
    }
    for (size_t i = 0; fh->files_info[i]; i++)
    {
        struct mbt_str **path = fh->files_info[i]->path;
        for (size_t i = 0; path[i]; i++)
        {
            mbt_str_dtor(path[i]);
            free(path[i]);
        }
        free(fh->files_info[i]->path);
        free(fh->files_info[i]);
    }
    free(fh->files_info);
    free(fh->pieces);

    mbt_str_free(fh->h);
    mbt_str_free(fh->name);

    free(fh);
}
