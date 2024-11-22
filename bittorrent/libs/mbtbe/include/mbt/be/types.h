#ifndef TYPES_H
#define TYPES_H

#include <ctype.h>
#include <stdlib.h>
struct torrent_file
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    struct mbt_str *creation_date;
    struct info *info;
    struct mbt_str *path;
};

struct mbt_torrent
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    struct mbt_str *creation_date;
    struct info *info;
};

struct info
{
    int64_t piece_length;
    struct mbt_str pieces;
    struct mbt_str name;
    int64_t length;
    struct mbt_str sha1;
};

#endif // !TYPES_H
