#include <err.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

#include "arpa/inet.h"
#include "mbt/be/torrent.h"
#include "mbt/be/types_mbtbe.h"
#include "mbt/net/context.h"
#include "mbt/net/leeching.h"
#include "mbt/net/net_types.h"
#include "netinet/in.h"
#include "string.h"

int main(int argc, char *argv[])
{
    if (argc > 2
        && (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--mktorrent") == 0))
    {
        return mbt_be_make_torrent_file(argv[2]);
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
    mbt_torrent_print(torrent);

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
