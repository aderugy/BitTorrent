#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/net/context.h>
#include <mbt/net/leeching.h>
#include <stdlib.h>

#include "main.h"

int main_download(struct main_options options)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    if (!mbt_be_parse_torrent_file(options.path, torrent))
    {
        errx(EXIT_FAILURE, "main_download: mbt_be_parse_torrent_file");
    }

    struct in_addr ip;
    uint16_t port = atoi(options.port ? options.port : "6891");
    inet_pton(AF_INET, options.ip ? options.ip : "0.0.0.0", &ip);

    struct mbt_net_context *context = mbt_net_context_init(torrent, ip, port);
    if (!context)
    {
        errx(EXIT_FAILURE, "main_download: mbt_net_context_init");
    }

    mbt_leech(context);

    mbt_torrent_free(torrent);
    mbt_net_context_free(context);
    return options.flags;
}
