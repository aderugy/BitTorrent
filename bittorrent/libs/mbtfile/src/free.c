#include <mbt/be/torrent.h>
#include <mbt/file/file_types.h>
#include <stdlib.h>

void mbt_file_handler_free(struct mbt_file_handler *fh)
{
    if (!fh)
    {
        return;
    }

    if (fh->torrent)
    {
        mbt_torrent_free(fh->torrent);
    }

    free(fh);
}
