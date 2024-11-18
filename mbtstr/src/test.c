#include <mbtstr/str.h>
#include <stddef.h>

bool mbt_str_ctor(struct mbt_str *str, size_t capacity)
{
    return str->size && capacity;
}
