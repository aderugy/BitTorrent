#include <mbt/utils/file.h>

#include "mbt/utils/str.h"

bool mbt_str_read_file(const char *path, struct mbt_str *str)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return false;
    }
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1)
    {
        mbt_str_pushcstr(str, line);
    }
    return true;
}
