#ifndef TYPES_H
#define TYPES_H

#include <ctype.h>
#include <mbt/utils/str.h>
#include <stdlib.h>
struct mbt_torrent_file
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    size_t creation_date;
    struct info *info;
    struct mbt_str *path;
};

struct mbt_torrent
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    size_t creation_date;
    struct info *info;
};

struct info
{
    size_t piece_length;
    struct mbt_str pieces;
    struct mbt_str name;
    size_t length;
    struct mbt_str sha1;
    struct mbt_torrent_file **files;
};

#endif // !TYPES_H
