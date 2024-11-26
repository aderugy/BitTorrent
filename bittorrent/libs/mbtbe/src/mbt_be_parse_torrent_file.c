#include <mbt/be/bencode.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "mbt/be/torrent.h"
#include "mbt/utils/hash.h"
#include "stdio.h"

void push_file(struct mbt_torrent *torrent, struct mbt_be_node **path_list,
               size_t length)
{
    struct mbt_torrent_file **list = torrent->info->files;
    list = realloc(
        list, sizeof(struct mbt_torrent_file *) * (torrent->info->length + 2));
    torrent->info->files = list;

    size_t index = 0;
    while (list[index])
    {
        index++;
    }
    list[index] = calloc(1, sizeof(struct mbt_torrent_file));
    for (size_t j = 0; path_list[j]; j++)
    {
        list[index]->path =
            realloc(list[index]->path, sizeof(struct mbt_str *) * (j + 2));
        if (!path_list[j]->v.str.data)
        {
            errx(1, "mbt be parse torrent file : path is null");
            return;
        }
        struct mbt_str *str = calloc(1, sizeof(struct mbt_str));
        if (!mbt_str_ctor(str, 64))
        {
            errx(1, "mbt be parse torrent file : cannot init path");
            return;
        }
        if (!mbt_str_pushcstr(str, path_list[j]->v.str.data))
        {
            errx(1, "mbt be parse torrent file : cannot push path");
            return;
        }
        list[index]->path[j] = str;
        list[index]->path_size++;
        list[index]->path[j + 1] = NULL;
    }

    list[index]->length = length;
    list[index]->start_index = torrent->info->length;
    torrent->info->length += length;
    list[index + 1] = NULL;
}
bool handle_files_list(struct mbt_torrent *torrent, struct mbt_be_node *node,
                       size_t index)
{
    struct mbt_be_node *val = node->v.dict[index]->val;
    size_t i = 0;
    struct mbt_be_node **list = val->v.list;
    while (list[i])
    {
        struct mbt_be_node *file = list[i];
        struct mbt_be_node **list_path = file->v.dict[1]->val->v.list;

        size_t length = file->v.dict[0]->val->v.nb;

        push_file(torrent, list_path, length);
        i++;
    }
    return true;
}

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
        struct mbt_str str = mbt_be_encode(val);
        char *h = sha1(str.data, str.size);
        memcpy(torrent->info->hash, h, 20);

        mbt_str_dtor(&str);
        free(h);

        for (size_t i = 0; val->v.dict[i]; i++)
        {
            struct mbt_cview key = MBT_CVIEW_OF(val->v.dict[i]->key);
            struct mbt_be_node *nval = val->v.dict[i]->val;
            if (strcmp(key.data, "pieces") == 0)
            {
                mbt_str_pushcstr(torrent->info->pieces, nval->v.str.data);
            }
            else if (strcmp(key.data, "piece length") == 0)
            {
                torrent->info->piece_length = nval->v.nb;
            }
            else if (strcmp(key.data, "name") == 0)
            {
                mbt_str_pushcstr(torrent->info->name, nval->v.str.data);
            }
            else if (strcmp(key.data, "length") == 0)
            {
                torrent->info->length = nval->v.nb;
            }
            else if (strcmp(key.data, "files") == 0)
            {
                printf("handling files\n");
                handle_files_list(torrent, val, i);
            }
            else
            {
                errx(1, "Unknown key: %s", key.data);
            }
        }
        struct mbt_str node_string = mbt_be_encode(val);
        mbt_str_pushcstr(torrent->info->info_string, node_string.data);
        mbt_str_dtor(&node_string);
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

    printf("data -> %s\n", view.data);
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
