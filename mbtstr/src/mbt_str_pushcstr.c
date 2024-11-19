#include <mbtstr/str.h>

bool mbt_str_pushcstr(struct mbt_str *str, const char *cstr)
{
    if (!cstr)
    {
        return false;
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
