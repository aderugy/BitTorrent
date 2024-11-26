#include <mbt/be/bencode.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/str.h>
#include <stdio.h>
#include <stdlib.h>

void print_torrent(struct mbt_torrent *torrent)
{
    printf("created by -> %s\n", torrent->created_by->data);
    printf("announce -> %s\n", torrent->announce->data);
    printf("creation date -> %li\n", torrent->creation_date);
    printf("info ->\n");
    printf("--pieces -> %s\n", torrent->info->pieces->data);
    printf("--info string -> %s\n", torrent->info->info_string->data);
}

void test_parse(void)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    mbt_be_parse_torrent_file("torrents/bonapart.png.torrent", torrent);
    print_torrent(torrent);
    mbt_torrent_free(torrent);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }

    mbt_be_make_torrent_file(argv[1]);
    test_parse();
    return EXIT_SUCCESS;
}
