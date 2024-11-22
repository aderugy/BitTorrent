#include <mbt/be/torrent.h>
#include <mbt/utils/view.h>
#include <stdlib.h>
struct mbt_torrent
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    struct mbt_str *creation_date;
    struct info *info;
};
struct info
{
    unsigned int lenght;
    struct mbt_str *name;
    unsigned int piece_lenght;
    struct mbt_str *pieces;
};

struct mbt_torrent *mbt_torrent_init(void)
{
    struct mbt_torrent *torrent = malloc(sizeof(struct mbt_torrent));
    if (!torrent)
    {
        return NULL;
    }
    torrent->announce = NULL;
    torrent->created_by = NULL;
    torrent->creation_date = NULL;
    torrent->info = NULL;
    return torrent;
}
