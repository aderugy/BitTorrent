#include <mbt/utils/parse.h>
#include <stdlib.h>
#include <string.h>
#include <mbt/utils/str.h>
#include <err.h>

char *parse_path_get_file_name(const char *path)
{
    char *file_name = strrchr(path, '/');
    if (file_name == NULL)
    {
        return NULL;
    }
    return file_name + 1;
}

struct mbt_str *parse_path_get_dir_name(const char *path)
{
    char *copy = calloc(strlen(path), sizeof(char));
    for (size_t i = 0; *(path + i + 1); i++)
    {
        copy[i] = path[i];
    }

    char *dir_name = strrchr(copy, '/');
    if (dir_name == NULL)
    {
        return NULL;
    }
    struct mbt_str *path_mbt = calloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(path_mbt, 32))
    {
        errx(1, "parse path get dir name");
    }
    for (size_t h = 1; *(dir_name + h); h++)
    {
        if (!mbt_str_pushc(path_mbt, *(dir_name + h)))
        {
            errx(1, "pushc parse path get dir name");
        }
    }

    free(copy);

    return path_mbt;
}
