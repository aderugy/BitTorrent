#include <err.h>
#include <libgen.h>
#include <mbt/utils/parse.h>
#include <mbt/utils/str.h>
#include <stdlib.h>
#include <string.h>

#include "libgen.h"
#include "stdio.h"

char *parse_path_get_file_name(const char *path)
{
    char *copy = calloc(strlen(path) + 1, sizeof(char));

    for (size_t i = 0; *(path + i); i++)
    {
        copy[i] = path[i];
    }

    char *file_name = basename(copy);
    if (file_name == NULL)
    {
        return NULL;
    }

    return file_name;
}

struct mbt_str *parse_path_get_dir_name(const char *path)
{
    char *copy = calloc(strlen(path) + 1, sizeof(char));

    for (size_t i = 0; *(path + i); i++)
    {
        copy[i] = path[i];
    }

    char *dir_name = basename(copy);

    struct mbt_str *path_mbt = calloc(1, sizeof(struct mbt_str));

    if (!mbt_str_ctor(path_mbt, 32))
    {
        errx(1, "parse path get dir name");
    }

    for (size_t h = 0; *(dir_name + h); h++)
    {
        if (!mbt_str_pushc(path_mbt, *(dir_name + h)))
        {
            errx(1, "pushc parse path get dir name");
        }
    }

    free(copy);

    return path_mbt;
}
