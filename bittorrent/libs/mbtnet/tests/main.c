#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/net/context.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/peer.h>

int main(void)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    mbt_be_parse_torrent_file("./launch-me.torrent", torrent);

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

    struct in_addr ip = { 0 };
    struct mbt_net_context *ctx = mbt_net_context_init(torrent, ip, 8000);
    mbt_net_context_peers(ctx);

    struct mbt_net_client *clients = NULL;
    struct mbt_net_server *server = mbt_net_server_init(ctx);

    while (42)
    {
        mbt_net_server_process_event(server, &clients);
    }

    errx(1, "carre");
}
