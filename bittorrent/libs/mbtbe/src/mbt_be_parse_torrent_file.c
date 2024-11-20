#include <mbt/be/bencode.h>
#include <mbt/utils/str.h>

#include "mbt/be/torrent.h"

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

bool mbt_be_parse_torrent_file(const char *path, struct mbt_torrent *torrent)
{
    struct mbt_be *file = mbt_str_init(64);

    

    return true;
}
