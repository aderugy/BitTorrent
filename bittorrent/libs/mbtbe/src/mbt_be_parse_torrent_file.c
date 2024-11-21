#include <mbt/be/bencode.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
    struct mbt_str *data = mbt_str_init(64);
    if (!mbt_str_ctor(data, 64))
    {
        mbt_str_free(data);
        return false;
    }
    if (!mbt_str_read_file(path, data))
    {
        mbt_str_free(data);
        return false;
    }

    struct mbt_cview view = MBT_CVIEW_OF(*data);

    struct mbt_be_node *node = mbt_be_decode(&view);

    if (node->type != MBT_BE_DICT)
    {
        mbt_str_free(data);
        return false;
    }

    for (size_t i = 0; node->v.dict[i]; i++)
    {
        struct mbt_cview key = MBT_CVIEW_OF(node->v.dict[i]->key);
        struct mbt_be_node *val = node->v.dict[i]->val;
        if (strcmp(key.data, "announce") == 0)
        {
            torrent->announce = &val->v.str;
        }
        else if (strcmp(key.data, "created by") == 0)
        {
            torrent->created_by = &val->v.str;
        }
        else if (strcmp(key.data, "creation date") == 0)
        {
            torrent->creation_date = &val->v.str;
        }
        else if (strcmp(key.data, "info") == 0)
        {
            struct mbt_be_node *info = val;
            struct mbt_be_node *name = info->v.dict[0]->val;
            struct mbt_be_node *piece_lenght = info->v.dict[1]->val;
            struct mbt_be_node *pieces = info->v.dict[2]->val;
            torrent->info = malloc(sizeof(struct info));
            if (!torrent->info)
            {
                mbt_str_free(data);
                return false;
            }
            torrent->info->lenght = name->v.str.size;
            torrent->info->name = &name->v.str;
            torrent->info->piece_lenght = piece_lenght->v.nb;
            torrent->info->pieces = &pieces->v.str;
        }
    }

    return true;
}
