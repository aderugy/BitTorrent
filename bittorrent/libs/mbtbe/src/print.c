#include <mbt/be/torrent_getters.h>
#include <mbt/be/types_mbtbe.h>
#include <stdio.h>

void mbt_torrent_print(struct mbt_torrent *torrent, int mode)
{
    printf("{\n");
    printf("\t\"announce\" : \"%s\"\n", mbt_torrent_announce(torrent).data);

    printf("\t\"created_by\" : \"%s\"\n", mbt_torrent_created_by(torrent).data);
    printf("\t\"isdir\" : \"%s\"",
           mbt_torrent_is_dir(torrent) ? "true" : "false");
    printf("\n");
    printf("\t\"creation_date\" : %zu\n", torrent->creation_date);
    printf("\t\"info\" : \n\t{\n");

    printf("\t\t\"length : %zu\n", mbt_torrent_length(torrent));
    printf("\t\t\"name\" : \"%s\"\n", mbt_torrent_name(torrent).data);
    printf("\t\t\"pieces length\" : %zu\n", mbt_torrent_piece_length(torrent));
    printf("\t\t\"pieces\" : ");
    for (size_t i = 0; i < torrent->info->pieces->size; i++)
    {
        unsigned char c = torrent->info->pieces->data[i];
        printf("%02X", c);
    }
    printf("\n");

    if (mode == 1)
    {
        printf("\t\t\"files\" :\n");
        printf("\t\t{\n");
        for (size_t i = 0; torrent->info->files[i]; i++)
        {
            printf("\t\t\t\"length\" : %li\n", torrent->info->files[i]->length);
            printf("\t\t\t\"path\" : \"");
            for (size_t j = 0; torrent->info->files[i]->path[j]; j++)
            {
                printf("/%s", torrent->info->files[i]->path[j]->data);
            }
            printf("\"\n");
        }
        printf("\t\t}\n");
    }
    printf("\t}\n");
    printf("}\n");
    /*    printf("\t\t\"pieces\" : ");
        for (size_t i = 0; i < 20; i++)
        {
            unsigned char c = torrent->info->hash[i];
            printf("%02X", c);
        }*/
}
