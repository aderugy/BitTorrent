#include <mbt/be/torrent.h>
#include <mbt/utils/view.h>
#include <stdlib.h>

#include <mbt/be/types_mbtbe.h>
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
    return torrent;
}
