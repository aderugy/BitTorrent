#include <dirent.h>
#include <err.h>
#include <mbt/be/get_pairs.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/utils/file.h>
#include <mbt/utils/hash.h>
#include <mbt/utils/logger.h>
#include <mbt/utils/parse.h>
#include <mbt/utils/str.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mbt/be/bencode.h"
#include "mbt/utils/view.h"

static void get_pieces_string_buff(struct mbt_str data,
                                   struct mbt_str *sha1_mbt)
{
    for (size_t i = 0; i < data.size; i += 256 * 1024)
    {
        size_t remaining_size = data.size - i;

        if (remaining_size < 256 * 1024)
        {
            char *sha1_str = sha1(data.data + i, remaining_size);
            for (size_t i = 0; i < 20; i++)
            {
                if (!mbt_str_pushc(sha1_mbt, *(sha1_str + i)))
                {
                    errx(1, "sha1 pushc");
                }
            }
            free(sha1_str);
            break;
        }
        else
        {
            char *sha1_str = sha1(data.data + i, 256 * 1024);
            for (size_t i = 0; i < 20; i++)
            {
                if (!mbt_str_pushc(sha1_mbt, *(sha1_str + i)))
                {
                    errx(1, "sha1 pushc");
                }
            }
            free(sha1_str);
        }
    }
}

static void get_pieces_string(const char *path, struct mbt_str *sha1_mbt)
{
    struct mbt_str data;
    if (!mbt_str_ctor(&data, 64))
    {
        mbt_str_free(&data);
        return;
    }
    if (!mbt_str_read_file(path, &data))
    {
        mbt_str_free(&data);
        return;
    }
    for (size_t i = 0; i < data.size; i += 256 * 1024)
    {
        size_t remaining_size = data.size - i;

        if (remaining_size < 256 * 1024)
        {
            char *sha1_str = sha1(data.data + i, remaining_size);
            for (size_t i = 0; i < 20; i++)
            {
                if (!mbt_str_pushc(sha1_mbt, *(sha1_str + i)))
                {
                    errx(1, "sha1 pushc");
                }
            }
            free(sha1_str);
            break;
        }
        else
        {
            char *sha1_str = sha1(data.data + i, 256 * 1024);
            for (size_t i = 0; i < 20; i++)
            {
                if (!mbt_str_pushc(sha1_mbt, *(sha1_str + i)))
                {
                    errx(1, "sha1 pushc");
                }
            }
            free(sha1_str);
        }
    }
    mbt_str_dtor(&data);
}

static struct mbt_be_pair *get_user(const char *path)
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

static struct mbt_be_pair **add_to_dict(struct mbt_be_pair **d,
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

static struct mbt_be_node **add_to_list(struct mbt_be_node **l,
                                        struct mbt_be_node *node)
{
    if (l == NULL)
    {
        l = calloc(1, sizeof(struct mbt_be_node));
    }
    size_t size = 0;
    while (l[size] != NULL)
    {
        size++;
    }
    struct mbt_be_node **a =
        realloc(l, sizeof(struct mbt_be_node) * (size + 2));

    size_t i = 0;
    while (a[i] != NULL)
    {
        i++;
    }
    a[i] = node;
    a[i + 1] = NULL;
    return a;
}

static struct mbt_be_pair *get_path_of_file(char *path)
{
    struct mbt_be_node **l = calloc(1, sizeof(struct mbt_be_node));

    char *token = strtok(path, "/");

    while (token)
    {
        struct mbt_str str;
        if (!mbt_str_ctor(&str, 64))
        {
            errx(1, "get path of file");
        }
        if (!mbt_str_pushcstr(&str, token))
        {
            errx(1, "get path of file");
        }
        struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(str));
        l = add_to_list(l, node);
        token = strtok(NULL, "/");
        mbt_str_dtor(&str);
    }

    struct mbt_be_node *list = mbt_be_list_init(l);

    struct mbt_be_pair *pair_path = transform_to_pair(list, "path");

    return pair_path;
}

static struct mbt_be_node *dict_of_file_length_path(char *path,
                                                    const char *origin)
{
    size_t length = get_file_size(path);

    struct mbt_be_node *node_length = mbt_be_num_init(length);

    struct mbt_be_pair *pair_length = transform_to_pair(node_length, "length");

    struct mbt_be_pair **d = calloc(2 + 1, sizeof(struct mbt_be_pair));

    d[0] = pair_length;
    d[1] = get_path_of_file(path + strlen(origin));
    d[2] = NULL;

    struct mbt_be_node *node = mbt_be_dict_init(d);

    return node;
}

static void fill_pieces_and_file_length_rec(char *path, struct mbt_str *pieces,
                                            uint64_t *size)
{
    DIR *d = opendir(path);
    if (d == NULL)
    {
        return;
    }
    struct dirent **files;
    int n = scandir(path, &files, 0, alphasort);
    while (n--)
    {
        struct dirent *dir = files[n];
        if (dir->d_type == DT_REG)
        {
            char *file_path = calloc(1, strlen(path) + strlen(dir->d_name) + 2);
            strcat(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, dir->d_name);

            *size += get_file_size(file_path);

            struct mbt_str *sha1_mbt = calloc(1, sizeof(struct mbt_str));
            if (!mbt_str_ctor(sha1_mbt, 64))
            {
                errx(1, "fill pieces rec : cannot init sha1_mbt");
            }

            mbt_str_read_file(file_path, pieces);

            mbt_str_dtor(sha1_mbt);
            free(sha1_mbt);
            free(file_path);
        }
        else if (dir->d_type == DT_DIR)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                free(files[n]);
                continue;
            }
            char *new_path = calloc(1, strlen(path) + strlen(dir->d_name) + 2);
            strcat(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, dir->d_name);

            fill_pieces_and_file_length_rec(new_path, pieces, size);
            free(new_path);
        }
        free(files[n]);
    }
    free(files);
    closedir(d);
}

static void fill_d_files_rec(char *path, struct mbt_be_node ***d_files,
                             const char *origin)
{
    if (d_files == NULL)
    {
        d_files = calloc(1, sizeof(struct mbt_be_node));
    }

    DIR *d = opendir(path);
    if (d == NULL)
    {
        return;
    }
    struct dirent **files;
    int n = scandir(path, &files, 0, alphasort);
    while (n--)
    {
        struct dirent *dir = files[n];

        if (dir->d_type == DT_REG)
        {
            char *file_path =
                calloc(1, 1 + strlen(path) + strlen(dir->d_name) + 1);
            strcat(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, dir->d_name);
            struct mbt_be_node *node =
                dict_of_file_length_path(file_path, origin);
            free(file_path);
            *d_files = add_to_list(*d_files, node);
        }
        else if (dir->d_type == DT_DIR)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                free(files[n]);
                continue;
            }
            char *new_path = calloc(1, strlen(path) + strlen(dir->d_name) + 2);
            strcat(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, dir->d_name);

            fill_d_files_rec(new_path, d_files, origin);
            free(new_path);
        }
        free(files[n]);
    }
    free(files);
    closedir(d);
}

static struct mbt_be_pair *list_of_files(const char *path)
{
    struct mbt_be_node **d_files = calloc(1, sizeof(struct mbt_be_pair));
    char *copy_path = calloc(1, strlen(path) + 1);
    strcat(copy_path, path);

    fill_d_files_rec(copy_path, &d_files, path);

    struct mbt_be_node *list = mbt_be_list_init(d_files);

    free(copy_path);
    return transform_to_pair(list, "files");
}

void print_pair(struct mbt_be_pair *pair)
{
    logger("key -> %s\n", pair->key.data);
    logger("size -> %li\n", pair->key.size);
    if (pair->val->type == MBT_BE_STR)
    {
        logger("val -> %s\n", pair->val->v.str.data);
    }
}

struct mbt_be_pair *get_pieces_dir(const char *path, uint64_t *size)
{
    struct mbt_str pieces;
    if (!mbt_str_ctor(&pieces, 64))
    {
        mbt_str_free(&pieces);
        return NULL;
    }
    char *new_path = calloc(1, strlen(path) + 1);
    strcat(new_path, path);

    fill_pieces_and_file_length_rec(new_path, &pieces, size);

    struct mbt_str sha;
    if (!mbt_str_ctor(&sha, 64))
    {
        errx(1, "mbt_str_ctor");
    }
    get_pieces_string_buff(pieces, &sha);

    free(new_path);
    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(sha));
    struct mbt_be_pair *pair = transform_to_pair(node, "pieces");
    mbt_str_dtor(&pieces);
    mbt_str_dtor(&sha);
    return pair;
};
struct mbt_be_pair *get_pieces(const char *path)
{
    struct mbt_str sha1_mbt;
    if (!mbt_str_ctor(&sha1_mbt, 64))
    {
        mbt_str_free(&sha1_mbt);
        return NULL;
    }
    get_pieces_string(path, &sha1_mbt);

    struct mbt_be_node *node = mbt_be_str_init(MBT_CVIEW_OF(sha1_mbt));
    struct mbt_be_pair *pair = transform_to_pair(node, "pieces");
    mbt_str_dtor(&sha1_mbt);
    return pair;
}

struct mbt_be_pair *create_info_dict(const char *path)
{
    struct mbt_be_pair **d = calloc(1, sizeof(struct mbt_be_pair));

    if (!is_dir(path))
    {
        d = add_to_dict(d, get_length(path));
        d = add_to_dict(d, get_name(path));
        d = add_to_dict(d, get_pieces_length());
        d = add_to_dict(d, get_pieces(path));
    }
    else
    {
        d = add_to_dict(d, list_of_files(path));
        d = add_to_dict(d, get_name(path));
        uint64_t size = 0;
        d = add_to_dict(d, get_pieces_length());
        d = add_to_dict(d, get_pieces_dir(path, &size));
    }
    struct mbt_be_node *node = mbt_be_dict_init(d);
    struct mbt_be_pair *pair = transform_to_pair(node, "info");
    return pair;
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

    struct mbt_be_node *node = mbt_be_dict_init(d);

    if (node->type != MBT_BE_DICT)
    {
        errx(1, "mbt be make torent file not a dict");
    }

    struct mbt_str buffer = mbt_be_encode(node);
    FILE *fptr;
    struct mbt_str path_mbt;
    if (!mbt_str_ctor(&path_mbt, 10))
    {
        errx(1, "cant path mbt form make torrent");
    }
    if (is_dir(path))
    {
        struct mbt_str *dir_name = parse_path_get_dir_name(path);
        mbt_str_pushcstr(&path_mbt, dir_name->data);
        mbt_str_dtor(dir_name);
        free(dir_name);
    }
    else
    {
        char *file_name = parse_path_get_file_name(path);
        mbt_str_pushcstr(&path_mbt, file_name);
        free(file_name);
    }

    mbt_str_pushcstr(&path_mbt, ".torrent");
    fptr = fopen(path_mbt.data, "w");
    for (size_t i = 0; i < buffer.size; i++)
    {
        fputc(buffer.data[i], fptr);
    }
    fclose(fptr);

    mbt_str_dtor(&data);
    mbt_be_free(node);
    mbt_str_dtor(&buffer);
    mbt_str_dtor(&path_mbt);
    return true;
}
