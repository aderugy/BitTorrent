#include <mbt/be/bencode.h>
#include <mbt/utils/file.h>
#include <mbt/be/types.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stddef.h>
#include <string.h>

#include "err.h"
#include "mbt/be/torrent.h"

bool fill_torrent(struct mbt_torrent *torrent, struct mbt_be_node *node,
                  size_t index)
{
    struct mbt_cview key = MBT_CVIEW_OF(node->v.dict[index]->key);
    struct mbt_be_node *val = node->v.dict[index]->val;
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
        torrent->creation_date = val->v.nb;
    }
    else if (strcmp(key.data, "info"))
    {
        for (size_t i = 0; node->v.dict[i]; i++)
        {
            struct mbt_cview key = MBT_CVIEW_OF(node->v.dict[i]->key);
            struct mbt_be_node *val = node->v.dict[i]->val;
            if (strcmp(key.data, "pieces"))
            {
                torrent->info->pieces = val->v.str;
            }
            else if (strcmp(key.data, "piece length"))
            {
                torrent->info->piece_length = val->v.nb;
            }
            else if (strcmp(key.data, "name"))
            {
                torrent->info->name = val->v.str;
            }
            else if (strcmp(key.data, "length"))
            {
                torrent->info->length = val->v.nb;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool mbt_be_parse_torrent_file(const char *path, struct mbt_torrent *torrent)
{
    struct mbt_str *data = mbt_str_init(64);

    if (!mbt_str_ctor(data, 64))
    {
        errx(1, "mbt be parse torrent file : cannot init data\n");
        return false;
    }
    if (!mbt_str_read_file(path, data))
    {
        errx(1, "mbt be parse torrent file : cannot read the file\n");
        return false;
    }

    struct mbt_cview view = MBT_CVIEW_OF(*data);

    struct mbt_be_node *node = mbt_be_decode(&view);

    if (node->type != MBT_BE_DICT)
    {
        errx(1, "mbt be parse torrent file : the node is not a dict\n");
        return false;
    }

    for (size_t i = 0; node->v.dict[i]; i++)
    {
        if (!fill_torrent(torrent, node, i))
        {
            errx(1, "fill torrent did not parse successfully\n");
            return false;
        }
    }

    return true;
}
