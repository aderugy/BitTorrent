#include <dirent.h>
#include <err.h>
#include <mbt/be/get_pairs.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/hash.h>
#include <mbt/utils/parse.h>
#include <mbt/utils/str.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

bool is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);

    return S_ISDIR(path_stat.st_mode);
}

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
        errx(1, "get file size");
    }
    return st.st_size;
}
struct mbt_be_pair *get_pieces_length()
{
    struct mbt_be_node *node = mbt_be_num_init(256 * 1024);
    struct mbt_be_pair *pair = transform_to_pair(node, "piece length");
    return pair;
}
struct mbt_be_pair *get_name(const char *path)
{
    struct mbt_str name;
    if (!mbt_str_ctor(&name, 64))
    {
        errx(1, "failed to ctor mbt_be_pair");
        return NULL;
    }
    char *name_str;
    if (is_dir(path))
    {
        struct mbt_str *str = parse_path_get_dir_name(path);
        name_str = str->data;
        if (!mbt_str_pushcstr(&name, name_str))
        {
            mbt_str_free(&name);
            return NULL;
        }
        mbt_str_dtor(str);
        free(str);
    }
    else
    {
        name_str = parse_path_get_file_name(path);

        if (!mbt_str_pushcstr(&name, name_str))
        {
            mbt_str_free(&name);
            return NULL;
        }
    }

    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(name));
    struct mbt_be_pair *pair = transform_to_pair(node, "name");
    mbt_str_dtor(&name);
    return pair;
}
struct mbt_be_pair *get_length(const char *path)
{
    uint64_t size = get_file_size(path);
    struct mbt_be_node *node = mbt_be_num_init(size);
    struct mbt_be_pair *pair = transform_to_pair(node, "length");
    return pair;
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
    mbt_str_pushcstr(&announce, "http://localhost:6969/announce");
    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(announce));

    struct mbt_be_pair *pair = transform_to_pair(node, "announce");

    mbt_str_dtor(&announce);
    return pair;
}
