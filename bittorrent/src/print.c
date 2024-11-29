#include "main.h"
#include "mbt/be/torrent.h"
#include "mbt/be/types_mbtbe.h"

int main_prettyprint(struct main_options options)
{
    struct mbt_torrent *torrent = mbt_torrent_init();

    mbt_be_parse_torrent_file(options.path, torrent);

    mbt_torrent_print(torrent, 1);

    mbt_torrent_free(torrent);

    return options.flags;
}
