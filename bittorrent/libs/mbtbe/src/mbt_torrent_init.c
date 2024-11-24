#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/view.h>
#include <stdlib.h>

#include "err.h"
#include "mbt/utils/str.h"

struct mbt_torrent *mbt_torrent_init(void)
{
    struct mbt_torrent *torrent = calloc(1, sizeof(struct mbt_torrent));
    if (!torrent)
    {
        return NULL;
    }
    torrent->info = calloc(1, sizeof(struct info));
    if (torrent->info == NULL)
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
