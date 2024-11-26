#include <dirent.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/hash.h>
#include <mbt/utils/str.h>
#include <pwd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mbt/be/bencode.h"
#include "stdio.h"
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
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return st.st_size;
}

struct mbt_be_pair *get_pieces_length(void)
{
    struct mbt_be_node *node = mbt_be_num_init(256 * 1024);
    struct mbt_be_pair *pair = transform_to_pair(node, "piece length");
    return pair;
}
char *parse_path_get_file_name(const char *path)
{
    char *file_name = strrchr(path, '/');
    if (file_name == NULL)
    {
        return NULL;
    }
    return file_name + 1;
}

char *parse_path_get_dir_name(const char *path)
{
    char *copy = strdup(path);
    if (copy[strlen(copy) - 1] == '/')
    {
        copy[strlen(copy) - 1] = 0;
    }
    char *dir_name = strrchr(copy, '/');
    if (dir_name == NULL)
    {
        return NULL;
    }
    return dir_name + 1;
}

struct mbt_be_pair *get_name(const char *path)
{
    struct mbt_str name;
    if (!mbt_str_ctor(&name, 64))
    {
        mbt_str_free(&name);
        return NULL;
    }
    char *name_str;
    if (is_dir(path))
    {
        name_str = parse_path_get_dir_name(path);
    }
    else
    {
        name_str = parse_path_get_file_name(path);
    }
    if (!mbt_str_pushcstr(&name, name_str))
    {
        mbt_str_free(&name);
        return NULL;
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

struct mbt_be_pair *get_pieces(const char *path)
{
    struct mbt_str data;
    if (!mbt_str_ctor(&data, 64))
    {
        mbt_str_free(&data);
        return NULL;
    }
    if (!mbt_str_read_file(path, &data))
    {
        mbt_str_free(&data);
        return NULL;
    }

    struct mbt_str sha1_mbt;
    if (!mbt_str_ctor(&sha1_mbt, 64))
    {
        mbt_str_free(&sha1_mbt);
        return NULL;
    }

    size_t remaining_size = data.size;
    for (size_t i = 0; i < data.size; i += 256 * 1024)
    {
        if (remaining_size < 256 * 1024)
        {
            char *sha1_str = sha1(data.data + i, remaining_size);
            if (!mbt_str_pushcstr(&sha1_mbt, sha1_str))
            {
                mbt_str_free(&sha1_mbt);
                return NULL;
            }
            free(sha1_str);
            break;
        }
        else
        {
            char *sha1_str = sha1(data.data + i, 256 * 1024);
            if (!mbt_str_pushcstr(&sha1_mbt, sha1_str))
            {
                mbt_str_free(&sha1_mbt);
                return NULL;
            }
            free(sha1_str);
        }
    }

    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(sha1_mbt));
    struct mbt_be_pair *pair = transform_to_pair(node, "pieces");
    mbt_str_dtor(&data);
    mbt_str_dtor(&sha1_mbt);
    return pair;
}

struct mbt_be_pair *create_info_dict(const char *path)
{
    struct mbt_be_pair **d = calloc(3 + 1, sizeof(struct mbt_be_pair));
    d[0] = get_pieces_length();
    d[1] = get_name(path);
    d[2] = get_pieces(path);
    d[3] = get_length(path);
    d[4] = NULL;
    struct mbt_be_node *node = mbt_be_dict_init(d);
    struct mbt_be_pair *pair = transform_to_pair(node, "info");
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

struct mbt_be_pair **add_to_dict(struct mbt_be_pair **d,
                                 struct mbt_be_pair *pair)
{
    struct mbt_be_pair **a =
        realloc(d, sizeof(struct mbt_be_pair) * (sizeof(d) + 1));
    size_t i = 0;
    while (a[i] != NULL)
    {
        i++;
    }
    a[i] = pair;
    a[i + 1] = NULL;
    return a;
}

struct mbt_be_node **add_to_list(struct mbt_be_node **l,
                                 struct mbt_be_node *node)
{
    struct mbt_be_node **a =
        realloc(l, sizeof(struct mbt_be_node) * (sizeof(l) + 1));
    size_t i = 0;
    while (a[i] != NULL)
    {
        i++;
    }
    a[i] = node;
    a[i + 1] = NULL;
    return a;
}

struct mbt_be_node *dict_of_file_length_path(char *path)
{
    size_t length = get_file_size(path);
    struct mbt_be_node *node_length = mbt_be_num_init(length);
    struct mbt_str name;
    if (!mbt_str_ctor(&name, 64))
    {
        mbt_str_free(&name);
        return NULL;
    }
    if (!mbt_str_pushcstr(&name, parse_path_get_file_name(path)))
    {
        mbt_str_free(&name);
        return NULL;
    }
    struct mbt_be_node *node_name = mbt_be_str_init(MBT_CVIEW_OF(name));
    struct mbt_be_pair *pair_name = transform_to_pair(node_name, "name");
    struct mbt_be_pair *pair_length = transform_to_pair(node_length, "length");
    struct mbt_be_pair **d = calloc(2 + 1, sizeof(struct mbt_be_pair));
    d[0] = pair_name;
    d[1] = pair_length;
    d[2] = NULL;
    struct mbt_be_node *node = mbt_be_dict_init(d);
    return node;
}

struct mbt_be_pair *list_of_files(const char *path)
{
    DIR *d = opendir(path);
    if (d == NULL)
    {
        return NULL;
    }
    struct mbt_be_node **d_files = calloc(1, sizeof(struct mbt_be_pair));
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
        {
            char *file_path = calloc(1, strlen(path) + strlen(dir->d_name) + 1);
            strcat(file_path, path);
            strcat(file_path, dir->d_name);
            struct mbt_be_node *node = dict_of_file_length_path(file_path);
            d_files = add_to_list(d_files, node);
        }
    }

    struct mbt_be_node *list = mbt_be_list_init(d_files);

    closedir(d);

    return transform_to_pair(list, "files");
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

    struct mbt_be_pair **d = calloc(1, sizeof(struct mbt_be_pair));

    d = add_to_dict(d, get_announce());
    d = add_to_dict(d, get_user(path));
    d = add_to_dict(d, get_creation_date(path));
    d = add_to_dict(d, create_info_dict(path));
    if (is_dir(path))
    {
        d = add_to_dict(d, list_of_files(path));
    }

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
    if (is_dir(path))
    {
        char *dir_name = parse_path_get_dir_name(path);
        printf("dir_name -> %s\n", dir_name);
        mbt_str_pushcstr(&path_mbt, dir_name);
    }
    else
    {
        char *file_name = parse_path_get_file_name(path);
        mbt_str_pushcstr(&path_mbt, file_name);
    }

    mbt_str_pushcstr(&path_mbt, ".torrent");
    fptr = fopen(path_mbt.data, "w");
    fputs(buffer.data, fptr);
    fclose(fptr);

    mbt_str_dtor(&data);
    mbt_be_free(node);
    mbt_str_dtor(&buffer);
    mbt_str_dtor(&path_mbt);
    return true;
}
