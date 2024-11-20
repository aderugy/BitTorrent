#include <mbt/utils/view.h>
#include <mbt/utils/str.h>
#include <stddef.h>
#include <stdlib.h>

struct mbt_str *mbt_str_init(size_t capacity)
{
    struct mbt_str *str = calloc(1, sizeof(struct mbt_str));
    if (!str)
    {
        return NULL;
    }

    if (!mbt_str_ctor(str, capacity))
    {
        free(str);
        return NULL;
    }

    return str;
}
