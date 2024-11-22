#include <mbt/utils/str.h>
#include <mbt/utils/view.h>

#include "err.h"

bool mbt_str_pushcstr(struct mbt_str *str, const char *cstr)
{
    if (!cstr)
    {
        return false;
    }
    if (!str->size)
    {
        errx(1, "there is not size on the struct from pushcstr");
    }

    for (size_t i = 0; *(cstr + i); i++)
    {
        if (!mbt_str_pushc(str, *(cstr + i)))
        {
            return false;
        }
    }

    return true;
}
