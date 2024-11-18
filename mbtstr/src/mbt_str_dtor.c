#include "mbtstr/str.h"
#include "stdlib.h"

void mbt_str_dtor(struct mbt_str *str)
{
    free(str->data);
}
