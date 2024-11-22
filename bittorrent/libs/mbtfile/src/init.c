#include <mbt/be/types.h>
#include <mbt/file/file_types.h>
#include <stdlib.h>

struct mbt_file_handler *mbt_file_handler_init(struct mbt_torrent *torrent)
{
    struct mbt_file_handler *fh = calloc(1, sizeof(struct mbt_file_handler));
    if (!fh)
    {
        return NULL;
    }

    fh->torrent = torrent;
    return fh;
}
