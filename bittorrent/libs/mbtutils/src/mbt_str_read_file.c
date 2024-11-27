#include <mbt/utils/file.h>

#include "mbt/utils/str.h"
#include "stdio.h"

bool mbt_str_read_file(const char *path, struct mbt_str *str)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return false;
    }

    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        mbt_str_pushc(str, ch);
    }

    fclose(file);
    return true;
}
