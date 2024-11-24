#include <err.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stddef.h>
#include <stdlib.h>

struct mbt_str *mbt_str_init(size_t capacity)
{
    struct mbt_str *str = xcalloc(1, sizeof(struct mbt_str));
    if (!mbt_str_ctor(str, capacity))
    {
        errx(EXIT_FAILURE, "mbt_str_init: mbt_str_ctor failed");
    }

    return str;
}
