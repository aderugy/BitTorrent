#include <ctype.h>
#include <mbtstr/str.h>
#include <mbtstr/view.h>
#include <stdio.h>

void mbt_cview_fprint(struct mbt_cview view, FILE *stream)
{
    for (size_t i = 0; i < view.size; i++)
    {
        char c = view.data[i];

        if (isprint(c))
        {
            fprintf(stream, "%c", c);
        }
        else
        {
            fprintf(stream, "U+00%2X", c);
        }
    }
    fprintf(stream, "\n");
}
