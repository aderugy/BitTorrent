#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/str.h>
#include <stdlib.h>
#include "mbt/be/bencode.h"

void delocate_info(struct info *info)
{
    if (info->info_string)
    {
        mbt_str_free(info->info_string);
    }
    if (info->name)
    {
        mbt_str_free(info->name);
    }
    if (info->pieces)
    {
        mbt_str_free(info->pieces);
    }
    if (info->files)
    {
        for (size_t i = 0; info->files[i]; i++)
        {
            for (size_t j = 0; info->files[i]->path[j]; j++)
            {
                mbt_str_free(info->files[i]->path[j]);
            }
            free(info->files[i]->path);
            free(info->files[i]);
        }
        free(info->files);
    }
}

void mbt_torrent_free(struct mbt_torrent *torrent)
{
    if (torrent->info)
    {
        delocate_info(torrent->info);
        free(torrent->info);
    }
    if (torrent->announce)
    {
        mbt_str_free(torrent->announce);
    }
    if (torrent->created_by)
    {
        mbt_str_free(torrent->created_by);
    }
    if (torrent->root)
    {
        mbt_be_free(torrent->root);
    }
    free(torrent);
}
