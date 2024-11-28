#include <err.h>
#include <mbt/file/file_handler.h>
#include <mbt/file/file_types.h>
#include <mbt/file/piece.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mbt/utils/str.h"

char *create_path(struct mbt_str **path, size_t path_length)
{
    char *copy = calloc(64, sizeof(char));
    for (size_t i = 0; i < path_length - 1; i++)
    {
        mkdir(path[i]->data, 0777);
        strcat(copy, path[i]->data);
    }

    return copy;
}

bool write_in_file(const char *path, const char *start_data,
                   const char *end_data)
{
    FILE *file = fopen(path, "w");

    if (!file)
    {
        return false;
    }

    for (const char *i = start_data; i < end_data; i++)
    {
        fputc(*i, file);
    }

    return true;
}

bool mbt_piece_write(struct mbt_file_handler *fh, size_t piece_index)
{
    size_t read_piece_size = 0;
    for (size_t i = 0; fh->files_info[i]; i++)
    {
        char *path = create_path(fh->files_info[i]->path,
                                 fh->files_info[i]->path_length);
        if (!path)
        {
            return false;
        }

        const char *start_data =
            mbt_piece_get_data(fh, piece_index) + read_piece_size;
        const char *end_data = start_data + fh->files_info[i]->size;

        if (!write_in_file(path, start_data, end_data))
        {
            return false;
        }
    }
    return true;
}
