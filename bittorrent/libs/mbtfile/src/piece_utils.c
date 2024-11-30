#include <mbt/file/file_types.h>
#include <stdlib.h>
#include <string.h>

void mbt_piece_reset(struct mbt_piece *piece)
{
    if (piece->completed)
    {
        return;
    }

    if (piece->data)
    {
        free(piece->data);
        piece->data = NULL;
    }

    memset(piece->status, 0, MBT_PIECE_NB_BLOCK * sizeof(int));
}
