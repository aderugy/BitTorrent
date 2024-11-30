#include <mbt/be/torrent_getters.h>
#include <mbt/be/types_mbtbe.h>

#include "err.h"
#include "mbt/utils/str.h"
#include "mbt/utils/view.h"

struct mbt_cview copy(const struct mbt_cview str)
{
    struct mbt_str *cpy = mbt_str_init(64);
    if (!mbt_str_pushcv(cpy, str))
    {
        errx(1, "copy failed");
    }
    return MBT_CVIEW_OF(*cpy);
}

struct mbt_cview mbt_torrent_announce(const struct mbt_torrent *torrent)
{
    return MBT_CVIEW_OF(*torrent->announce);
}

struct mbt_cview mbt_torrent_created_by(const struct mbt_torrent *torrent)
{
    return MBT_CVIEW_OF(*torrent->created_by);
}

size_t mbt_torrent_creation_date(const struct mbt_torrent *torrent)
{
    return torrent->creation_date;
}

size_t mbt_torrent_piece_length(const struct mbt_torrent *torrent)
{
    return torrent->info->piece_length;
}

struct mbt_cview mbt_torrent_name(const struct mbt_torrent *torrent)
{
    return MBT_CVIEW_OF(*torrent->info->name);
}

size_t mbt_torrent_length(const struct mbt_torrent *torrent)
{
    return torrent->info->length;
}

const struct mbt_be_node *mbt_torrent_node(const struct mbt_torrent *torrent)
{
    return torrent->root;
}
