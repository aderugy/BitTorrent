#include <mbt/utils/str.h>
#include <mbt/utils/view.h>

#include <stdlib.h>

bool mbt_str_ctor(struct mbt_str *str, size_t capacity)
{
    str->data = calloc(capacity + 1, sizeof(char));
    str->size = 0;
    if (str->data == NULL)
    {
        return false;
    }

    str->capacity = capacity;
    return true;
}
