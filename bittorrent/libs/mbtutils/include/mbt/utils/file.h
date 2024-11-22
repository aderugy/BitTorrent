#ifndef FILE_H
#define FILE_H

#include <mbt/utils/str.h>
#include <stdbool.h>

bool mbt_str_read_file(const char *path, struct mbt_str *str);

#endif // !FILE_H
