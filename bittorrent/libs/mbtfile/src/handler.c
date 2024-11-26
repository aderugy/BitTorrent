#include <err.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/view.h>

size_t mbt_file_handler_get_total_size(
    __attribute((unused)) struct mbt_file_handler *fh)
{
    warnx("mbt_file_handler_get_total_size: undefined behavior for multiple "
          "files");
    return MBT_PIECE_SIZE * fh->nb_pieces;
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
