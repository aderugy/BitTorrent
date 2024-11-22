#include <mbt/be/torrent.h>
#include <mbt/utils/str.h>
#include <stdlib.h>

struct torrent_file
{
    struct mbt_str *announce;
    struct mbt_str *created_by;
    struct mbt_str *creation_date;
    struct info *info;
};

bool init_announce(struct mbt_be_node *node, struct torrent_file *torrent)
{
    if (node->type != MBT_BE_STR)
    {
        return false;
    }
    torrent->announce = &node->v.str;
    return true;
}

bool mbt_be_make_torrent_file(const char *path)
{
    struct torrent_file *torrent = malloc(sizeof(struct torrent_file));
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

    struct mbt_be_node *node = mbt_be_dict_init(NULL);

    if (node->type != MBT_BE_DICT)
    {
        mbt_str_free(data);
        return false;
    }

    if (!init_announce(node->v.dict[0]->val, torrent))
    {
        mbt_str_free(data);
        return false;
    }

    return true;
}
