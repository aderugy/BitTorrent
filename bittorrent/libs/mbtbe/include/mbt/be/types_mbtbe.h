#ifndef TYPES_MBTBE_H
#define TYPES_MBTBE_H

#include <mbt/utils/str.h>
#include <stddef.h>

#include "mbt/be/bencode.h"
struct mbt_torrent_file
{
    struct mbt_str **path;
    size_t length;
    size_t path_size;
    size_t start_index;
};

struct mbt_torrent
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    size_t creation_date;
    struct info *info;
    struct mbt_be_node *root;
    bool is_dir;
};

struct info
{
    char hash[20];

    size_t piece_length;
    struct mbt_str *pieces;
    struct mbt_str *name;
    size_t length;
    struct mbt_str *sha1;
    struct mbt_torrent_file **files;
    struct mbt_str *info_string;
};

void mbt_torrent_print(struct mbt_torrent *torrent, int mode);
struct mbt_be_node *mbt_be_dict_get(struct mbt_be_node *dict, const char *key);
void pretty_print_torrent(struct mbt_torrent *torrent);

#endif // !TYPES_MBTBE_H
