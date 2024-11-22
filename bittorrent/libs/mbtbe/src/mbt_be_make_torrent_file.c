#include <mbt/utils/file.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types.h>
#include <mbt/utils/str.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mbt/be/bencode.h"
#include "stdio.h"
#include "unistd.h"

struct mbt_be_pair *transform_to_pair(struct mbt_be_node *node, char *value)
{
    struct mbt_str *key = mbt_str_init(64);
    if (!mbt_str_ctor(key, 64))
    {
        mbt_str_free(key);
        return NULL;
    }
    if (!mbt_str_pushcstr(key, value))
    {
        mbt_str_free(key);
        return NULL;
    }
    struct mbt_be_pair *pair = mbt_be_pair_init(MBT_CVIEW_OF(*key), node);
    return pair;
}

uint64_t get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) != 0)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return st.st_size;
}

struct mbt_be_pair *get_creation_date(struct mbt_torrent_file *torrent)
{
    struct mbt_be_node *node = mbt_be_num_init(0);
    if (node->type != MBT_BE_NUM)
    {
        return NULL;
    }

    struct mbt_str *path = torrent->path;

    if (path->size == 0)
    {
        return NULL;
    }

    struct stat st;
    if (stat(path->data, &st) != 0)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    node->v.nb = st.st_mtime;
    node->type = MBT_BE_NUM;

    struct mbt_be_pair *pair = transform_to_pair(node, "creation date");
    return pair;
}

struct mbt_be_pair *get_user(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    struct passwd *pw = getpwuid(st.st_uid);
    if (pw == NULL)
    {
        return NULL;
    }
    struct mbt_str *user = mbt_str_init(64);
    if (!mbt_str_ctor(user, 64))
    {
        mbt_str_free(user);
        return NULL;
    }
    if (!mbt_str_pushcstr(user, pw->pw_name))
    {
        mbt_str_free(user);
        return NULL;
    }
    struct mbt_cview user_cv = MBT_CVIEW_OF(*user);
    struct mbt_be_node *node = mbt_be_str_init(user_cv);

    struct mbt_be_pair *pair = transform_to_pair(node, "created by");
    return pair;
}

bool mbt_be_make_torrent_file(const char *path)
{
    struct mbt_str *data = mbt_str_init(64);
    if (!mbt_str_ctor(data, 64))
    {
        errx(1, "make torrent file : cannot init data");
        return false;
    }
    if (!mbt_str_read_file(path, data))
    {
        errx(1, "make torrent file : cannot read file %s\n", path);
        return false;
    }

    struct mbt_be_pair **d = calloc(1 + 1, sizeof(struct mbt_be_pair));
    d[0] = get_user(path);

    struct mbt_be_node *node = mbt_be_dict_init(d);

    if (node->type != MBT_BE_DICT)
    {
        errx(1, "mbt be make torent file not a dict");
        return false;
    }

    struct mbt_str buffer = mbt_be_encode(node);
    FILE *fptr;
    fptr = fopen("path", "w");
    fputs(buffer.data, fptr);
    mbt_be_free(node);

    return true;
}
