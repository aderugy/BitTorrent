#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>

bool mbt_str_ctor(struct mbt_str *str, size_t capacity)
{
    if (str->data)
    {
        free(str->data);
    }

    str->data = xcalloc(capacity + 1, sizeof(char));
    str->capacity = capacity;
    str->size = 0;

    return true;
}
