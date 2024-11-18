#include "mbtstr/str.h"
#include "stdlib.h"

bool mbt_str_ctor(struct mbt_str *str, size_t capacity)
{
    char *data = calloc(capacity, sizeof(char));
    if (data == NULL)
    {
        return false;
    }
    str->data = data;
    str->capacity = capacity;
    str->size = 0;
    return true;
}
