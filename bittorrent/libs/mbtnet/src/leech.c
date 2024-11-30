#include <err.h>
#include <mbt/file/file_types.h>
#include <mbt/net/context.h>
#include <mbt/net/fifo.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/peer.h>
#include <mbt/utils/logger.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static void find_blocks(struct mbt_net_client **clients,
                        struct mbt_net_server *server)
{
    struct mbt_net_client *head = *clients;
    while (head)
    {
        int status = mbt_net_client_next_block(server, head);
        int fd = head->fd;

        head = head->next;

        if (status == STREAM_NO_BLOCK_AVAILABLE)
        {
            mbt_net_clients_remove(server, clients, fd, true);
        }
    }

    if (!*clients)
    {
        struct mbt_file_handler *fh = server->ctx->fh;

        for (size_t i = 0; i < fh->nb_pieces; i++)
        {
            if (!fh->pieces[i]->completed)
            {
                mbt_net_server_free(server);
                errx(EXIT_FAILURE, "dl failed");
            }
        }

        mbt_net_server_free(server);
        exit(0);
    }
}

void mbt_leech(struct mbt_net_context *ctx)
{
    if (ctx->left == 0)
    {
        return;
    }

    // Get list of peers to contact
    struct mbt_peer **peers = mbt_net_context_peers(ctx);
    if (!peers)
    {
        errx(EXIT_FAILURE, "mbt_leech: peers or server");
    }

    struct mbt_net_client *clients = NULL;
    struct mbt_net_server *server = mbt_net_server_init(ctx);

    for (size_t i = 0; peers[i]; i++)
    {
        struct mbt_peer *peer = peers[i];
        mbt_net_peer_connect(server, &clients, peer);
    }

    for (size_t i = 0; peers[i]; i++)
    {
        mbt_peer_free(peers[i]);
    }
    free(peers);

    while (true)
    {
        find_blocks(&clients, server);

        for (size_t i = 0; i < server->streams->size; i++)
        {
            struct mbt_net_stream *stream = fifo_pop(server->streams);

            int status;
            if (stream->status == STREAM_READY
                && (status = mbt_msg_send_handler_request(server, stream))
                    != MBT_HANDLER_SUCCESS)
            {
                warnx("mbt_msg_send_handler_request: failed");
            }
            else if (stream->status != STREAM_DONE)
            {
                fifo_push(server->streams, stream);
            }
        }

        mbt_net_server_process_event(server, &clients);
    }
}
