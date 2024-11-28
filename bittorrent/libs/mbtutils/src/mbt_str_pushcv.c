#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stdbool.h>
#include <stddef.h>

bool mbt_str_pushcv(struct mbt_str *str, struct mbt_cview view)
{
    for (size_t i = 0; i < view.size; i++)
    {
        if (!mbt_str_pushc(str, view.data[i]))
        {
            return false;
        }
    }

    return true;
}
