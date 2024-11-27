#include <mbt/be/torrent_getters.h>
#include <mbt/be/types_mbtbe.h>
#include <stdlib.h>

char *format_hexa_pieces(struct mbt_cview pieces)
{
    char *formatted_pieces = calloc(pieces.size * 2 + 1, sizeof(char));
    for (size_t i = 0; i < pieces.size; i++)
    {
        sprintf(formatted_pieces + i * 2, "%02x", pieces.data[i]);
    }
    return formatted_pieces;
}

void pretty_print_torrent(struct mbt_torrent *torrent)
{
    printf("{\n");
    printf("    \"announce\" : \"%s\",\n", mbt_torrent_announce(torrent).data);
    printf("    \"created_by\" : \"%s\",\n",
           mbt_torrent_created_by(torrent).data);
    printf("    \"creation_date\" : %zu,\n",
           mbt_torrent_creation_date(torrent));
    printf("    \"info\" : {\n");
    printf("        \"length\" : %zu,\n", mbt_torrent_length(torrent));
    printf("        \"name\" : \"%s\",\n", mbt_torrent_name(torrent).data);
    printf("        \"piece_length\" : %zu,\n",
           mbt_torrent_piece_length(torrent));
    char *formatted_pieces = format_hexa_pieces(mbt_torrent_pieces(torrent));
    printf("        \"pieces\" : \"%s\",\n", formatted_pieces);
    free(formatted_pieces);
    printf("    }\n");
    printf("}\n");
}
