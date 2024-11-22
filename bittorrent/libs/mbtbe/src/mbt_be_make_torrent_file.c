#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/str.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mbt/be/bencode.h"
#include "stdio.h"

struct mbt_be_pair *transform_to_pair(struct mbt_be_node *node, char *value)
{
    struct mbt_str key;
    if (!mbt_str_ctor(&key, 64))
    {
        mbt_str_free(&key);
        return NULL;
    }
    if (!mbt_str_pushcstr(&key, value))
    {
        mbt_str_free(&key);
        return NULL;
    }
    struct mbt_be_pair *pair = mbt_be_pair_init(MBT_CVIEW_OF(key), node);
    mbt_str_dtor(&key);
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

struct mbt_be_pair *get_creation_date(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    size_t i = st.st_mtime;
    struct mbt_be_node *node = mbt_be_num_init(i);

    struct mbt_be_pair *pair = transform_to_pair(node, "creation date");
    return pair;
}

struct mbt_be_pair *get_announce(void)
{
    struct mbt_str announce;
    if (!mbt_str_ctor(&announce, 64))
    {
        errx(1, "get announce : cannot ctor");
        return NULL;
    }
    mbt_str_pushcstr(&announce,
                     "http://torrent.pie.cri.epita.fr:8000/announce");
    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(announce));

    struct mbt_be_pair *pair = transform_to_pair(node, "announce");

    mbt_str_dtor(&announce);
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
    struct mbt_str user;
    if (!mbt_str_ctor(&user, 64))
    {
        mbt_str_free(&user);
        return NULL;
    }
    if (!mbt_str_pushcstr(&user, pw->pw_name))
    {
        mbt_str_free(&user);
        return NULL;
    }
    struct mbt_cview user_cv = MBT_CVIEW_OF(user);
    struct mbt_be_node *node = mbt_be_str_init(user_cv);

    struct mbt_be_pair *pair = transform_to_pair(node, "created by");
    mbt_str_dtor(&user);
    return pair;
}

void print_pair(struct mbt_be_pair *pair)
{
    printf("key -> %s\n", pair->key.data);
    printf("size -> %li\n", pair->key.size);
    if (pair->val->type == MBT_BE_STR)
    {
        printf("val -> %s\n", pair->val->v.str.data);
    }
}

bool mbt_be_make_torrent_file(const char *path)
{
    struct mbt_str data;
    if (!mbt_str_ctor(&data, 64))
    {
        errx(1, "make torrent file : cannot init data");
        return false;
    }
    if (!mbt_str_read_file(path, &data))
    {
        errx(1, "make torrent file : cannot read file %s\n", path);
        return false;
    }

    struct mbt_be_pair **d = calloc(3 + 1, sizeof(struct mbt_be_pair));
    d[0] = get_announce();
    d[1] = get_user(path);
    d[2] = get_creation_date(path);
    d[3] = NULL;

    struct mbt_be_node *node = mbt_be_dict_init(d);

    if (node->type != MBT_BE_DICT)
    {
        errx(1, "mbt be make torent file not a dict");
        return false;
    }

    struct mbt_str buffer = mbt_be_encode(node);
    FILE *fptr;
    struct mbt_str path_mbt;
    if (!mbt_str_ctor(&path_mbt, 10))
    {
        errx(1, "cant path mbt form make torrent");
        return false;
    }
    mbt_str_pushcstr(&path_mbt, path);
    mbt_str_pushcstr(&path_mbt, ".torrent");
    fptr = fopen(path_mbt.data, "w");
    fputs(buffer.data, fptr);
    fclose(fptr);

    mbt_str_dtor(&data);
    mbt_be_free(node);
    mbt_str_dtor(&buffer);
    return true;
}
