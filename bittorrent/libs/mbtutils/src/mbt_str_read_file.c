#include <mbt/utils/file.h>

#include "mbt/utils/str.h"

bool mbt_str_read_file(const char *path, struct mbt_str *str)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return false;
    }
    char line[100];
    while (fgets(line, 100, file))
    {
        mbt_str_pushcstr(str, line);
    }
    return true;
}
