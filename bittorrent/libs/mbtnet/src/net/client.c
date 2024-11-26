#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mbt/net/msg_handler.h"

const char *mbt_client_state_name(enum mbt_client_state state)
{
    static const char *client_states[10] = { "MBT_CLIENT_WAITING_CONNECTION",
                                             "MBT_CLIENT_CONNECTED",
                                             "MBT_CLIENT_WAITING_HANDSHAKE",
                                             "MBT_CLIENT_HANDSHAKEN",
                                             "MBT_CLIENT_BITFIELD_RECEIVED",
                                             "MBT_CLIENT_REQUESTING",
                                             "MBT_CLIENT_DOWNLOADED",
                                             "MBT_CLIENT_COMPLETED",
                                             "MBT_CLIENT_ERROR",
                                             "MBT_CLIENT_DISCONNECTED" };

    return client_states[state];
}

void mbt_net_clients_print(struct mbt_net_client *clients)
{
    if (!clients)
    {
        return;
    }

    printf("CLIENT %hhi (%s)\n", clients->fd,
           mbt_client_state_name(clients->state));
    printf("-- START BUFFER (%zu) --\n", clients->read);

    for (size_t i = 0; i < clients->read; i++)
    {
        unsigned char c = clients->buffer[i];
        printf("%02X ", c);
    }
    printf("\n\n");

    mbt_net_clients_print(clients->next);
}

int mbt_net_client_handshake(struct mbt_net_server *server,
                             struct mbt_net_client *client)
{
    struct mbt_msg_handshake hs;
    mbt_msg_write_handshake(server->ctx, &hs);

    int status = sendall(client->fd, &hs, sizeof(struct mbt_msg_handshake));
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

    struct addrinfo *addr = server->addr;
    mbt_peer_init_addr(peer);
    if (!peer->addr)
    {
        return false;
    }

    struct addrinfo *next = addr;
    while (next)
    {
        addr = next;
        next = addr->ai_next;

        int c_fd =
            socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (c_fd < 0)
        {
            continue;
        }

        int flags = fcntl(c_fd, F_GETFL, 0);
        fcntl(c_fd, F_SETFL, flags | O_NONBLOCK);

        enum mbt_client_state state = MBT_CLIENT_WAITING_CONNECTION;

        int cstatus =
            connect(c_fd, peer->addr->ai_addr, peer->addr->ai_addrlen);

        if (cstatus && errno != EINPROGRESS)
        {
            return false;
        }
        else if (cstatus == 0)
        {
            state = MBT_CLIENT_CONNECTED;
        }

        if (!mbt_net_clients_add(server, clients, c_fd, state))
        {
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
    printf("Removing client...\n");
    if (!*clients)
    {
        return false;
    }

    if ((*clients)->fd != client_fd)
    {
        return mbt_net_clients_remove(server, &((*clients)->next), client_fd,
                                      close_fd);
    }

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
