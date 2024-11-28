#include <mbt/net/piece_handler.h>
#include <stdlib.h>

#include "mbt/file/file_types.h"
#include "mbt/net/fifo.h"
#include "mbt/net/net.h"
#include "mbt/utils/xalloc.h"

struct mbt_piece_handler *mbt_piece_handler_init(struct mbt_file_handler *fh)
{
    struct mbt_piece_handler *ph = xcalloc(1, sizeof(struct mbt_piece_handler));

    ph->nb_trackers = fh->nb_pieces;
    ph->trackers = xcalloc(ph->nb_trackers, sizeof(struct mbt_piece_tracker *));

    for (size_t i = 0; i < ph->nb_trackers; i++)
    {
        struct mbt_piece_tracker *tracker =
            xcalloc(1, sizeof(struct mbt_piece_tracker));

        tracker->piece = fh->pieces[i];
        tracker->clients = fifo_init();
        ph->trackers[i] = tracker;
    }

    return ph;
}

void mbt_piece_handler_free(struct mbt_piece_handler *ph)
{
    for (size_t i = 0; i < ph->nb_trackers; i++)
    {
        free(ph->trackers[i]);
    }
    free(ph->trackers);
    free(ph);
}

void mbt_piece_handler_add_client(struct mbt_piece_handler *ph,
                                  struct mbt_net_client *client)
{
    for (size_t i = 0; i < ph->nb_trackers; i++)
    {
        struct mbt_piece_tracker *tracker = ph->trackers[i];

        if (client->bitfield[i])
        {
            fifo_push(tracker->clients, client);
        }
    }
}

void mbt_piece_handler_remove_client(struct mbt_piece_handler *ph,
                                     struct mbt_net_client *client)
{
    for (size_t i = 0; i < ph->nb_trackers; i++)
    {
        struct mbt_piece_tracker *tracker = ph->trackers[i];

        if (client->bitfield[i])
        {
            if (tracker->client == client)
            {
                tracker->client = NULL;
                mbt_piece_reset(tracker->piece);
            }

            for (size_t j = 0; j < tracker->clients->size; j++)
            {
                struct mbt_net_client *subscribed = fifo_pop(tracker->clients);

                if (subscribed != client)
                {
                    fifo_push(tracker->clients, subscribed);
                }
            }
        }
    }
}

int mbt_piece_handler_find(struct mbt_piece_handler *ph)
{
    size_t nb_clients_dl = 0;

    size_t rarest_idx = ph->nb_trackers;
    size_t rarest_nb = 0;

    int status = PIECE_HANDLER_FULL;
    for (size_t i = 0; i < ph->nb_trackers; i++)
    {
        struct mbt_piece_tracker *tracker = ph->trackers[i];
        struct mbt_piece *piece = tracker->piece;

        if (piece->completed || nb_clients_dl >= 4)
        {
            continue;
        }

        if (tracker->client)
        {
            nb_clients_dl++;
            if (nb_clients_dl >= 4)
            {
                return PIECE_HANDLER_FULL;
            }

            continue;
        }

        if (tracker->clients->size == 0)
        {
            status = PIECE_HANDLER_UNREACHABLE;
        }

        for (size_t j = 0; j < tracker->clients->size; j++)
        {
            struct mbt_net_client *client = fifo_head(tracker->clients);

            if (client->state == MBT_CLIENT_READY && !client->choked
                && (rarest_idx == ph->nb_trackers
                    || rarest_nb > tracker->clients->size))
            {
                rarest_idx = i;
                rarest_nb = tracker->clients->size;
                status = PIECE_HANDLER_SUCCESS;
                break;
            }

            fifo_push(tracker->clients, fifo_pop(tracker->clients));
        }
    }

    return status;
}
