#include <err.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/str.h>
#include <stdio.h>
#include <stdlib.h>

#include "arpa/inet.h"
#include "mbt/be/torrent.h"
#include "mbt/net/context.h"
#include "mbt/net/leeching.h"
#include "mbt/net/net_types.h"
#include "netinet/in.h"
#include "string.h"

void verify_path(char **path)
{
    if (path == NULL)
    {
        errx(EXIT_FAILURE, "main: path");
    }
    if (*path[0] != '.')
    {
        char *new_path = calloc(strlen(*path) + 3, sizeof(char));
        new_path[0] = '.';
        new_path[1] = '/';
        strcat(new_path, *path);
        if (new_path == NULL)
        {
            errx(EXIT_FAILURE, "main: new_path");
        }
        *path = new_path;
    }
}

int main(int argc, char *argv[])
{
    if (argc > 2
        && (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--mktorrent") == 0))
    {
        verify_path(&argv[2]);
        if (!mbt_be_make_torrent_file(argv[2]))
        {
            errx(1, "mbt_be_make_torrent_file");
        }
        return 0;
    }
    else if (argc > 2
             && (strcmp(argv[1], "-P") == 0
                 || strcmp(argv[1], "--pretty-print-torrent-file") == 0))
    {
        verify_path(&argv[2]);
        struct mbt_torrent *torrent = mbt_torrent_init();
        if (!mbt_be_parse_torrent_file(argv[2], torrent))
        {
            errx(EXIT_FAILURE, "mbt_be_parse_torrent_file");
        }
        mbt_torrent_print(torrent, 0);
        mbt_torrent_free(torrent);
        return 0;
    }
    else if (argc > 2
             && (strcmp(argv[1], "-PF") == 0
                 || strcmp(argv[1], "--pretty-print-torrent-files") == 0))
    {
        verify_path(&argv[2]);
        struct mbt_torrent *torrent = mbt_torrent_init();
        if (!mbt_be_parse_torrent_file(argv[2], torrent))
        {
            errx(EXIT_FAILURE, "mbt_be_parse_torrent_file");
        }
        mbt_torrent_print(torrent, 1);
        mbt_torrent_free(torrent);
        return 0;
    }
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
    mbt_torrent_print(torrent, 0);

    struct in_addr ip;
    inet_pton(AF_INET, "127.0.0.1", &ip);

    struct mbt_net_context *ctx = mbt_net_context_init(torrent, ip, 8001);
    mbt_net_context_print(ctx);

    printf("MBT_NET_LEECH\n");
    mbt_leech(ctx);
    printf("END MBT_NET_LEECH\n\n");

    mbt_net_context_free(ctx);
    mbt_torrent_free(torrent);
    return 0;
}
