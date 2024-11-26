#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/view.h>
#include <stdlib.h>

#include "err.h"
#include "mbt/utils/str.h"

bool allocate_info(struct info *info)
{
    if (!info)
    {
        return false;
    }

    info->pieces = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(info->pieces, 64))
    {
        errx(1, "allocate_info -> failed to allocate pieces");
    }

    info->name = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(info->name, 64))
    {
        errx(1, "allocate_info -> failed to allocate name");
    }

    info->info_string = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(info->info_string, 64))
    {
        errx(1, "allocate_info -> failed to allocate info string");
    }

    info->files = calloc(1, sizeof(struct mbt_torrent_file *));

    return true;
}

struct mbt_torrent *mbt_torrent_init(void)
{
    struct mbt_torrent *torrent = calloc(1, sizeof(struct mbt_torrent));
    if (!torrent)
    {
        return NULL;
    }
    torrent->info = calloc(1, sizeof(struct info));
    if (!allocate_info(torrent->info))
    {
        free(torrent);
        return NULL;
    }

    torrent->announce = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(torrent->announce, 64))
    {
        errx(1, "failed to load announce (torrent init)");
    }

    torrent->created_by = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(torrent->created_by, 64))
    {
        errx(1, "failed to load created_by (torrent init)");
    }
    return torrent;
}
