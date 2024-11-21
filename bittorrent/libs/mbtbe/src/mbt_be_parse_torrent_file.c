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

    struct mbt_cview *view = calloc(1, sizeof(struct mbt_cview));
    if (!view)
    {
        mbt_str_free(data);
        return false;
    }
    view->data = data->data;
    view->size = data->size;

    struct mbt_be_node *node = mbt_be_decode(view);

    if (node->type != MBT_BE_DICT)
    {
        mbt_str_free(data);
        free(view);
        return false;
    }

    for (size_t i = 0; node->v.dict[i]; i++)
    {
        char *key = node->v.dict[i]->key.data;
        if (strcmp("announce", key) == 0)
        {
            torrent->announce = &node->v.dict[i]->val->v.str;
        }
    }

    return true;
}
