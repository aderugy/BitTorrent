#include "mbtstr/str.h"
#include "stdlib.h"

bool mbt_str_ctor(struct mbt_str *str, size_t capacity)
{
    str->data = calloc(capacity, sizeof(char));
    if (str->data == NULL)
    {
        return false;
    }

    str->capacity = capacity;
    return true;
}
