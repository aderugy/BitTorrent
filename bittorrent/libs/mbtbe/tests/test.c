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
    for (size_t i = 0; torrent->info->files[i]; i++)
    {
        printf("--file %li ->\n", i);
        printf("----length -> %li\n", torrent->info->files[i]->length);
        printf("----path -> ");
        for (size_t j = 0; torrent->info->files[i]->path[j]; j++)
        {
            printf("/%s", torrent->info->files[i]->path[j]->data);
        }
        printf("\n");
    }
}

void test_parse(char *filename)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    mbt_be_parse_torrent_file(filename, torrent);
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
    test_parse("torrents/bonapart.png.torrent");
    test_parse("torrents/bass_boosted1.mp4.torrent");
    test_parse("torrents/CongratulationsDir.torrent");
    test_parse("torrents/docu1.mp4.torrent");
    return EXIT_SUCCESS;
}
