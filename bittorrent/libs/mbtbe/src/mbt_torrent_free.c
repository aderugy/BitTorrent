#include <mbt/be/torrent.h>
#include <mbt/be/types.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

void mbt_torrent_free(struct mbt_torrent *torrent)
{
    if (torrent->announce)
    {
        mbt_str_free(torrent->announce);
    }
    if (torrent->created_by)
    {
        mbt_str_free(torrent->created_by);
    }

    if (torrent->info)
    {
            mbt_str_free(&torrent->info->name);
            mbt_str_free(&torrent->info->pieces);
    }
    free(torrent->info);
    free(torrent);
}
