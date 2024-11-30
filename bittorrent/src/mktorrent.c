#include "main.h"
#include "mbt/be/torrent.h"

int main_mktorrent(struct main_options options)
{
    mbt_be_make_torrent_file(options.path);

    return options.flags;
}
