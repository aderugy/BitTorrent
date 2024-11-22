#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

void mbt_torrent_free(struct mbt_torrent *torrent)
{
    if (torrent->info)
    {
        free(torrent->info);
    }
    free(torrent);
}
