#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/net/context.h>
#include <mbt/net/net_types.h>
#include <mbt/net/peer.h>

int main(void)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    mbt_be_parse_torrent_file("./launch-me.torrent", torrent);

    printf("Announce: %s\n", torrent->announce->data);
    printf("info->piece_length: %zu\n", torrent->info->piece_length);
    printf("info->pieces: %s\n", torrent->info->pieces.data);
    printf("info->name: %s\n", torrent->info->name.data);
    printf("info->length: %zu\n", torrent->info->length);
    printf("info->sha1: %s\n", torrent->info->sha1.data);

    errx(1, "carre");
}
