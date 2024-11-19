#include "mbtstr/str.h"
#include "stdlib.h"

bool mbt_str_pushc(struct mbt_str *str, char c)
{
    if (c == 0)
    {
        return true;
    }

    if (str->capacity == str->size)
    {
        size_t new_capacity = str->capacity + str->capacity / 2;

        char *tmp = realloc(str->data, new_capacity + 1);
        if (tmp == NULL)
        {
            return false;
        }

        str->data = tmp;
        str->capacity = new_capacity;
    }

    str->data[str->size] = c;
    str->size++;

    str->data[str->size] = 0;
    return true;
}
