#include <mbt/be/torrent_getters.h>
#include <mbt/file/file_handler.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stddef.h>
#include <string.h>

#include "err.h"
#include "mbt/be/types_mbtbe.h"
#include "stdio.h"

void copy_files_dir(struct mbt_file_handler *file_handler,
                    struct mbt_torrent *torrent)
{
    size_t s = 0;

    while (torrent->info->files[s])
    {
        s++;
    }

    file_handler->files_info = xcalloc(s + 1, sizeof(struct mbt_files_info *));

    for (size_t i = 0; torrent->info->files[i]; i++)
    {
        file_handler->files_info[i] = xcalloc(1, sizeof(struct mbt_files_info));

        struct mbt_files_info *info = file_handler->files_info[i];
        struct mbt_torrent_file *file = torrent->info->files[i];
        info->path = xcalloc(file->path_size + 1, sizeof(struct mbt_str *));
        info->size = file->length;
        size_t p = 0;
        for (p = 0; file->path[p]; p++)
        {
            info->path[p] = xcalloc(1, sizeof(struct mbt_str));
            struct mbt_str *str = info->path[p];
            if (!mbt_str_ctor(str, 64))
            {
                errx(1, "ctor copy file");
            }
            if (!mbt_str_pushcstr(str, file->path[p]->data))
            {
                errx(1, "pushcstr");
            }
        }
        info->path_length = p;
    }
}

void copy_file_single(struct mbt_file_handler *file_handler,
                      struct mbt_torrent *torrent)
{
    file_handler->files_info = xcalloc(2, sizeof(struct mbt_files_info *));
    file_handler->files_info[0] = xcalloc(1, sizeof(struct mbt_files_info));

    struct mbt_files_info *info = file_handler->files_info[0];

    info->size = torrent->info->length;
    info->path_length = 1;

    struct mbt_str *str = xcalloc(1, sizeof(struct mbt_str));

    if (!mbt_str_ctor(str, 64))
    {
        errx(1, "ctor");
    }

    if (!mbt_str_pushcstr(str, torrent->info->name->data))
    {
        errx(1, "puscstr");
    }
    info->path = xcalloc(2, sizeof(struct mbt_str));
    info->path[0] = str;
}

struct mbt_file_handler *mbt_file_handler_init(struct mbt_torrent *torrent)
{
    struct mbt_file_handler *fh = xcalloc(1, sizeof(struct mbt_file_handler));

    // Duplicate and copy
    struct mbt_cview hash = mbt_torrent_pieces(torrent);
    fh->h = mbt_str_init(hash.size);
    mbt_str_pushcv(fh->h, hash);

    // Duplicate name and copy
    struct mbt_cview name = mbt_torrent_name(torrent);
    fh->name = mbt_str_init(name.size);
    mbt_str_pushcv(fh->name, name);

    fh->nb_pieces = hash.size / 20;

    fh->pieces = xcalloc(fh->nb_pieces, sizeof(struct mbt_piece *));
    for (size_t i = 0; i < fh->nb_pieces; i++)
    {
        struct mbt_piece *piece = xcalloc(1, sizeof(struct mbt_piece));

        piece->h = xcalloc(MBT_H_LENGTH + 1, sizeof(char));

        // Store hash in piece
        memcpy(piece->h, hash.data + (i * MBT_H_LENGTH), MBT_H_LENGTH);

        if (i == fh->nb_pieces - 1) // Last piece can have smaller len
        {
            piece->size = torrent->info->length % MBT_PIECE_SIZE;
            if (piece->size == 0)
            {
                piece->size = MBT_PIECE_SIZE;
            }

            piece->nb_blocks = piece->size / MBT_BLOCK_SIZE
                + (piece->size % MBT_BLOCK_SIZE > 0);
        }
        else
        {
            piece->size = MBT_PIECE_SIZE;
            piece->nb_blocks = MBT_PIECE_NB_BLOCK;
        }

        fh->pieces[i] = piece;
    }
    if (torrent->is_dir)
    {
        copy_files_dir(fh, torrent);
    }
    else
    {
        copy_file_single(fh, torrent);
    }

    return fh;
}
