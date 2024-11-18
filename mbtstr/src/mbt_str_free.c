#include <mbtstr/str.h>
#include <stdlib.h>

void mbt_str_free(struct mbt_str *str)
{
    if (str)
    {
        mbt_str_dtor(str);
        free(str);
    }
}
