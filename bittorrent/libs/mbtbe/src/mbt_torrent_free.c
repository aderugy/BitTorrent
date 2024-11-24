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
    if (torrent->announce)
    {
        mbt_str_free(torrent->announce);
    }
    if (torrent->created_by) {
        mbt_str_free(torrent->created_by);
    
    }
    free(torrent);
}
