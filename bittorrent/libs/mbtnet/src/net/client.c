#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/logger.h>
#include <mbt/utils/xalloc.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bits/stdint-uintn.h"
#include "mbt/file/file_handler.h"
#include "mbt/file/file_types.h"
#include "mbt/file/piece.h"
#include "mbt/net/fifo.h"
#include "mbt/net/msg_handler.h"

const char *mbt_client_state_name(enum mbt_client_state state)
{
    static const char *client_states[8] = {
        "MBT_CLIENT_WAITING_CONNECTION", "MBT_CLIENT_CONNECTED",
        "MBT_CLIENT_WAITING_HANDSHAKE",  "MBT_CLIENT_HANDSHAKEN",
        "MBT_CLIENT_BITFIELD_RECEIVED",  "MBT_CLIENT_READY",
        "MBT_CLIENT_DOWNLOADING",        "MBT_CLIENT_COMPLETED",
    };

    return client_states[state];
}

void mbt_net_clients_print(struct mbt_net_client *clients)
{
    if (!clients)
    {
        return;
    }

    logger("CLIENT %hhi (%s)\n", clients->fd,
           mbt_client_state_name(clients->state));
    logger("-- START BUFFER (%zu) --\n", clients->read);

    for (size_t i = 0; i < clients->read; i++)
    {
        unsigned char c = clients->buffer[i];
        logger("%02X ", c);
    }
    logger("\n\n");

    mbt_net_clients_print(clients->next);
}

int mbt_net_client_next_block(struct mbt_net_server *server,
                              struct mbt_net_client *client)
{
    if (server->streams->size >= MAX_STREAMS_CONCURRENT
        || client->state != MBT_CLIENT_READY || client->choked)
    {
        return STREAM_FULL;
    }

    struct mbt_file_handler *fh = server->ctx->fh;
    for (uint32_t piece_index = 0; piece_index < fh->nb_pieces; piece_index++)
    {
        if (!client->bitfield[piece_index])
        {
            continue;
        }

        struct mbt_piece *piece = fh->pieces[piece_index];
        for (uint32_t block_index = 0; block_index < piece->nb_blocks;
             block_index++)
        {
            if (piece->status[block_index] == BLOCK_STATUS_FREE)
            {
                struct mbt_net_stream *stream =
                    xcalloc(1, sizeof(struct mbt_net_stream));

                stream->index = piece_index;
                stream->begin = block_index * MBT_BLOCK_SIZE;
                stream->length = stream->begin + MBT_BLOCK_SIZE > piece->size
                    ? piece->size % MBT_BLOCK_SIZE
                    : MBT_BLOCK_SIZE;

                piece->status[block_index] = BLOCK_STATUS_DL;

                stream->client = client;
                fifo_push(server->streams, stream);
                return STREAM_SUCCESS;
            }
        }
    }

    return STREAM_NO_BLOCK_AVAILABLE;
}

int mbt_net_client_handshake(struct mbt_net_server *server,
                             struct mbt_net_client *client)
{
    struct mbt_msg_handshake hs;
    mbt_msg_write_handshake(server->ctx, &hs);

    logger("handshake start\n");
    mbt_net_clients_print(client);
    int status = sendall(client->fd, &hs, sizeof(struct mbt_msg_handshake));
    logger("handshake end\n");
    if (status != MBT_HANDLER_SUCCESS)
    {
        MBT_HANDLER_STATUS(status) // Return appropriate mbt handler status
    }

    client->state = MBT_CLIENT_WAITING_HANDSHAKE;
    return MBT_HANDLER_SUCCESS;
}

bool mbt_net_client_check_connect(struct mbt_net_client *client)
{
    int err = 0;
    socklen_t len = sizeof(err);

    if (getsockopt(client->fd, SOL_SOCKET, SO_ERROR, &err, &len))
    {
        warnx("mbt_net_client_check_connect: Connection failed");
        return false;
    }

    client->state = MBT_CLIENT_CONNECTED;
    return true;
}

bool mbt_net_clients_add(struct mbt_net_server *server,
                         struct mbt_net_client **clients, int c_fd,
                         enum mbt_client_state state)
{
    // Register the client to the epoll instance
    struct epoll_event event = { .events = EPOLLIN | EPOLLOUT,
                                 .data = { .fd = c_fd } };

    if (epoll_ctl(server->ep_fd, EPOLL_CTL_ADD, c_fd, &event) == -1)
    {
        warnx("epoll_ctl");
        return false;
    }

    // Init new client
    struct mbt_net_client *next = xcalloc(1, sizeof(struct mbt_net_client));
    next->fd = c_fd;
    next->next = *clients;
    next->choked = true;
    next->state = state;
    *clients = next;

    return true;
}

bool mbt_net_peer_connect(struct mbt_net_server *server,
                          struct mbt_net_client **clients,
                          struct mbt_peer *peer)
{
    if (strcmp(server->ctx->ip, peer->ip->data) == 0
        && strcmp(server->ctx->port, peer->port->data) == 0)
    {
        warnx("Skipping our client...\n");
        return false;
    }

    logger("Peer: %s:%s\n", peer->ip->data, peer->port->data);

    mbt_peer_init_addr(peer);
    if (!peer->addr)
    {
        return false;
    }

    int c_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (c_fd < 0)
    {
        perror(NULL);
        warnx("socket: failed");
        return false;
    }

    struct addrinfo *addr = peer->addr;
    struct addrinfo *next = peer->addr;
    while (next)
    {
        addr = next;
        next = addr->ai_next;

        int cstatus = connect(c_fd, addr->ai_addr, addr->ai_addrlen);

        if (cstatus)
        {
            perror(NULL);
            warnx("connect: failed %d", errno);
            continue;
        }

        if (!mbt_net_clients_add(server, clients, c_fd, MBT_CLIENT_CONNECTED))
        {
            perror(NULL);
            warnx("mbt_net_clients_add: failed");
            close(c_fd);
            continue;
        }

        return true;
    }

    return false;
}

bool mbt_net_clients_remove(struct mbt_net_server *server,
                            struct mbt_net_client **clients, int client_fd,
                            bool close_fd)
{
    if (!*clients)
    {
        return false;
    }

    if ((*clients)->fd != client_fd)
    {
        return mbt_net_clients_remove(server, &((*clients)->next), client_fd,
                                      close_fd);
    }

    logger("Removing client %d (error = %d)\n", client_fd, close_fd);
    struct mbt_net_client *current = *clients;
    struct mbt_net_client *next = current->next;
    if (current->buffer)
    {
        free(current->buffer);
    }
    if (current->bitfield)
    {
        free(current->bitfield);
    }

    if (close_fd)
    {
        close(current->fd);
    }
    free(current);

    *clients = next;
    return true;
}

struct mbt_net_client *mbt_net_clients_find(struct mbt_net_client *clients,
                                            int client_fd)
{
    if (!clients)
    {
        return NULL;
    }

    if (clients->fd == client_fd)
    {
        return clients;
    }

    return mbt_net_clients_find(clients->next, client_fd);
}
