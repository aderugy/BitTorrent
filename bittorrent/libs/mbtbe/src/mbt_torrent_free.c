#include <mbt/be/torrent.h>
#include <mbt/utils/str.h>
#include <stddef.h>
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

void mbt_torrent_free(struct mbt_torrent *torrent)
{
    if (torrent->announce)
    {
        mbt_str_free(torrent->announce);
    }
    if (torrent->created_by)
    {
        mbt_str_free(torrent->created_by);
    }
    if (torrent->creation_date)
    {
        mbt_str_free(torrent->creation_date);
    }
    if (torrent->info)
    {
        if (torrent->info->name)
        {
            mbt_str_free(torrent->info->name);
        }
        if (torrent->info->pieces)
        {
            mbt_str_free(torrent->info->pieces);
        }
    }
    free(torrent->info);
    free(torrent);
}
