#include <err.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/view.h>

size_t mbt_file_handler_get_total_size(struct mbt_file_handler *fh)
{
    if (fh->nb_pieces == 0)
    {
        return 0;
    }

    return (fh->nb_pieces - 1) * MBT_PIECE_SIZE
        + (fh->pieces[fh->nb_pieces - 1]->size);
}

struct mbt_cview mbt_file_handler_get_name(struct mbt_file_handler *fh)
{
    return MBT_CVIEW_OF(*fh->name);
}

size_t
mbt_file_handler_get_nb_files(__attribute((unused)) struct mbt_file_handler *fh)
{
    warnx("mbt_file_handler_get_nb_files: undefined behavior for multiple "
          "files");
    return 1;
}
