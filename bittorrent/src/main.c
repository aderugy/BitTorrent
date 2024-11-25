#include <err.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

#include "mbt/be/torrent.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        errx(EXIT_FAILURE, "main: args");
    }

    char *path = argv[1];

    struct mbt_torrent *torrent = mbt_torrent_init();
    if (!mbt_be_parse_torrent_file(path, torrent))
    {
        errx(EXIT_FAILURE, "mbt_be_parse_torrent_file");
    }

    mbt_torrent_free(torrent);
    return 0;
}
