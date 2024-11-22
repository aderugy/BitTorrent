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
    struct mbt_str announce;
    if (!mbt_str_ctor(&announce, 64))
    {
        errx(1, "failed to load announce (torrent init)");
    }
    torrent->announce = &announce;

    struct mbt_str created_by;
    if (!mbt_str_ctor(&created_by, 64))
    {
        errx(1, "failed to load created_by (torrent init)");
    }
    torrent->created_by = &created_by;
    return torrent;
}
