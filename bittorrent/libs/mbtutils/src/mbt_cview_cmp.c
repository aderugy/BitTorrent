#include <mbt/utils/view.h>
#include <stddef.h>

int mbt_cview_cmp(struct mbt_cview lhs, struct mbt_cview rhs)
{
    size_t i = 0;
    for (; i < lhs.size; i++)
    {
        if (i >= rhs.size)
        {
            return 1;
        }

        int lc = lhs.data[i];
        int rc = rhs.data[i];

        if (lc != rc)
        {
            return lc - rc;
        }
    }

    if (i < rhs.size)
    {
        return -1;
    }

    return 0;
}
