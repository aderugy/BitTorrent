#include "mbtstr/str.h"
#include "stdlib.h"

bool mbt_str_pushc(struct mbt_str *str, char c)
{
    if (str->capacity == str->size)
    {
        size_t new_capacity = str->capacity *= 2;

        char *tmp = realloc(str->data, new_capacity + 1);
        if (tmp == NULL)
        {
            return false;
        }

        str->data = tmp;
        str->capacity = new_capacity;
    }

    str->data[str->size] = c;
    str->data[str->size + 1] = 0;
    str->size++;

    return true;
}
