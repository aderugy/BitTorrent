#include "mbtstr/view.h"

bool mbt_cview_contains(struct mbt_cview view, char c)
{
    for (size_t i = 0; *(view.data + i) != 0; i++)
    {
        if (*(view.data + i) == c)
        {
            return true;
        }
    }
    return false;
}
