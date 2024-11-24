#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/net/context.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/peer.h>
#include <stdlib.h>
#include <string.h>

#include "mbt/utils/str.h"

struct mbt_str *create_str(const char *s)
{
    struct mbt_str *out = mbt_str_init(strlen(s));
    mbt_str_pushcstr(out, s);

    return out;
}

struct mbt_torrent *get_torrent()
{
    struct mbt_torrent *torrent = calloc(1, sizeof(struct mbt_torrent));
    torrent->announce =
        create_str("http://torrent.pie.cri.epita.fr:8000/announce");
    torrent->created_by = create_str("ACU 2025");
    torrent->creation_date = 1731408914;

    struct info *info = calloc(1, sizeof(struct info));
    info->piece_length = 262144;
    info->length = 31504;

    struct mbt_str name = {
        .data = "launch-me",
        .size = 9,
        .capacity = 9,
    };

    struct mbt_str h = {
        .data = "hhhhhhhhhhhhhhhhhhhh",
        .size = 20,
        .capacity = 20,
    };

    info->name = name;
    info->pieces = h;

    torrent->info = info;
    return torrent;
}

int main(void)
{
    struct mbt_torrent *torrent = get_torrent();

    printf("Announce: %s\n", torrent->announce->data);
    printf("info->piece_length: %zu\n", torrent->info->piece_length);

    printf("info->pieces: ");
    struct mbt_str pieces = torrent->info->pieces;
    for (size_t i = 0; i < pieces.size; i++)
    {
        unsigned char c = pieces.data[i];
        printf("%02X", c);
    }
    printf("\n");

    printf("info->name: %s\n", torrent->info->name.data);
    printf("info->length: %zu\n", torrent->info->length);

    struct in_addr ip;
    inet_pton(AF_INET, "127.0.0.1", &ip);

    struct mbt_net_context *ctx = mbt_net_context_init(torrent, ip, 6969);
    mbt_net_context_peers(ctx);

    struct mbt_net_client *clients = NULL;
    struct mbt_net_server *server = mbt_net_server_init(ctx);

    while (42)
    {
        mbt_net_server_process_event(server, &clients);
    }

    errx(1, "carre");
}
