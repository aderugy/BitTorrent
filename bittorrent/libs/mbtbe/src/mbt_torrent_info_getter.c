#include <mbt/be/torrent_getters.h>
#include <mbt/be/types_mbtbe.h>

#include "mbt/utils/view.h"

const struct mbt_torrent_file *
mbt_torrent_files_get(const struct mbt_torrent *torrent, size_t idx)
{
    size_t i;
    for (i = 0; i < idx; i++)
    {
        if (!torrent->info->files[i])
        {
            return NULL;
        }
    }
    return torrent->info->files[i];
}

struct mbt_cview mbt_torrent_pieces(const struct mbt_torrent *torrent)
{
    return MBT_CVIEW_OF(*torrent->info->pieces);
}
size_t mbt_torrent_files_size(const struct mbt_torrent *torrent)
{
    size_t i = 0;
    while (torrent->info->files[i])
    {
        i++;
    }
    return i;
}
bool mbt_torrent_is_dir(const struct mbt_torrent *torrent)
{
    return torrent->is_dir;
}
