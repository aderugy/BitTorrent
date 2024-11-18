#include "mbtstr/str.h"
#include "stdlib.h"

bool mbt_str_pushc(struct mbt_str *str, char c)
{
    if (str->capacity == str->size)
    {
        str->data = realloc(str->data, str->capacity + 1);
        if (str->data == NULL)
        {
            return false;
        }
    }
    str->data[str->size] = c;
    str->size++;
    return true;
}
