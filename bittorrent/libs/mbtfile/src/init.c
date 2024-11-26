#include <mbt/be/torrent_getters.h>
#include <mbt/file/file_types.h>
#include <mbt/utils/str.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <string.h>

struct mbt_file_handler *mbt_file_handler_init(struct mbt_torrent *torrent)
{
    struct mbt_file_handler *fh = xcalloc(1, sizeof(struct mbt_file_handler));

    struct mbt_cview hash = mbt_torrent_pieces(torrent);
    fh->h = mbt_str_init(hash.size);
    mbt_str_pushcv(fh->h, hash);

    struct mbt_cview name = mbt_torrent_name(torrent);
    fh->name = mbt_str_init(name.size);
    mbt_str_pushcv(fh->name, name);

    fh->nb_pieces = hash.size / 20;

    fh->pieces = xcalloc(fh->nb_pieces, sizeof(struct mbt_piece *));
    for (size_t i = 0; i < fh->nb_pieces; i++)
    {
        struct mbt_piece *piece = xcalloc(1, sizeof(struct mbt_piece));

        piece->h = xcalloc(MBT_H_LENGTH + 1, sizeof(char));
        memcpy(piece->h, hash.data + (i * MBT_H_LENGTH), MBT_H_LENGTH);

        fh->pieces[i] = piece;
    }

    return fh;
}
