#include <mbt/be/types_mbtbe.h>
#include <stdio.h>

void mbt_torrent_print(struct mbt_torrent *torrent)
{
    printf("MBT_TORRENT\n");
    printf("\tAnnounce: %s\n", torrent->announce->data);
    printf("\tName    : %s\n", torrent->info->name->data);

    printf("\tPieces  : ");
    for (size_t i = 0; i < torrent->info->pieces->size; i++)
    {
        unsigned char c = torrent->info->pieces->data[i];
        printf("%02X", c);
    }
    printf("\n");

    printf("\tPieces length : %zu\n", torrent->info->piece_length);
    printf("\tFile   length : %zu\n", torrent->info->length);

    printf("\tInfo Hash     : ");
    for (size_t i = 0; i < 20; i++)
    {
        unsigned char c = torrent->info->hash[i];
        printf("%02X", c);
    }
    printf("\n\n");
}
