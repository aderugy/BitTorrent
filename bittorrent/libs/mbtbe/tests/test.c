#include <mbt/be/bencode.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/str.h>
#include <stdio.h>
#include <stdlib.h>

void print_torrent(struct mbt_torrent *torrent)
{
    printf("created by -> %s\n", torrent->created_by->data);
}

void test_parse(void)
{
    struct mbt_torrent *torrent = mbt_torrent_init();
    mbt_be_parse_torrent_file("./test.torrent", torrent);
    print_torrent(torrent);
    mbt_torrent_free(torrent);
}

int main(void)
{
    FILE *fptr = fopen("test", "w");
    fputc('a', fptr);
    fclose(fptr);

    struct mbt_str a;
    mbt_str_ctor(&a, 10);

    mbt_be_make_torrent_file("./test");
    mbt_str_dtor(&a);
    test_parse();
    return EXIT_SUCCESS;
}
