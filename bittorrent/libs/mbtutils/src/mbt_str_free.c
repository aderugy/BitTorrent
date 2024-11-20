#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <stdlib.h>

void mbt_str_free(struct mbt_str *str)
{
    if (str)
    {
        mbt_str_dtor(str);
        free(str);
    }
}
