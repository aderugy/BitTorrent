#ifndef PIECE_HANDLER_H
#define PIECE_HANDLER_H

#include <mbt/file/piece.h>
#include <mbt/net/fifo.h>
#include <mbt/net/net.h>

struct mbt_piece_tracker
{
    struct mbt_piece *piece;
    struct mbt_net_client *client;

    struct fifo *clients;
};

struct mbt_piece_handler
{
    struct mbt_file_handler *fh;

    size_t nb_trackers;
    struct mbt_piece_tracker **trackers;
};

struct mbt_piece_handler *mbt_piece_handler_init(struct mbt_file_handler *fh);
void mbt_piece_handler_free(struct mbt_piece_handler *ph);

void mbt_piece_handler_remove_client(struct mbt_piece_handler *ph,
                                     struct mbt_net_client *client);
void mbt_piece_handler_add_client(struct mbt_piece_handler *ph,
                                  struct mbt_net_client *client);

#define PIECE_HANDLER_SUCCESS 0
#define PIECE_HANDLER_FULL 1
#define PIECE_HANDLER_UNREACHABLE 2
int mbt_piece_handler_find(struct mbt_piece_handler *ph);

#endif // !PIECE_HANDLER_H
