#include <mbt/be/bencode.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stddef.h>
#include <string.h>

#include "err.h"
#include "mbt/be/torrent.h"
#include "stdio.h"

bool fill_torrent(struct mbt_torrent *torrent, struct mbt_be_node *node,
                  size_t index)
{
    struct mbt_cview key = MBT_CVIEW_OF(node->v.dict[index]->key);
    struct mbt_be_node *val = node->v.dict[index]->val;
    
    if (strcmp(key.data, "announce") == 0)
    {
        mbt_str_pushcstr(torrent->announce, val->v.str.data);
    }
    else if (strcmp(key.data, "created by") == 0)
    {
        mbt_str_pushcstr(torrent->created_by, val->v.str.data);
    }
    else if (strcmp(key.data, "creation date") == 0)
    {
        torrent->creation_date = val->v.nb;
    }
    else if (strcmp(key.data, "info") == 0)
    {
        for (size_t i = 0; val->v.dict[i]; i++)
        {
            struct mbt_cview key = MBT_CVIEW_OF(val->v.dict[i]->key);
            struct mbt_be_node *nval = val->v.dict[i]->val;
            if (strcmp(key.data, "pieces") == 0)
            {
                torrent->info->pieces = nval->v.str;
            }
            else if (strcmp(key.data, "piece length") == 0)
            {
                torrent->info->piece_length = nval->v.nb;
            }
            else if (strcmp(key.data, "name") == 0)
            {
                torrent->info->name = nval->v.str;
            }
            else if (strcmp(key.data, "length") == 0)
            {
                torrent->info->length = nval->v.nb;
            }
            else
            {
                errx(1, "Unknown key: %s", key.data);
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
    struct mbt_str data;

    if (!mbt_str_ctor(&data, 64))
    {
        errx(1, "mbt be parse torrent file : cannot init data");
        return false;
    }
    if (!mbt_str_read_file(path, &data))
    {
        errx(1, "mbt be parse torrent file : cannot read the file");
        return false;
    }

    struct mbt_cview view = MBT_CVIEW_OF(data);

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

    mbt_be_free(node);
    mbt_str_dtor(&data);

    return true;
}
