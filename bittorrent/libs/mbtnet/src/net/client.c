#include <ctype.h>
#include <err.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/xalloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

void mbt_net_clients_print(struct mbt_net_client *clients)
{
    if (!clients)
    {
        return;
    }

    printf("CLIENT %hhi (%s)\n", clients->fd,
           clients->handshaked ? "READY" : "PENDING");
    printf("-- START BUFFER (%zu) --\n", clients->read);
    for (size_t i = 0; i < clients->read; i++)
    {
        if (isprint(clients->buffer[i]))
        {
            printf("%c", clients->buffer[i]);
        }
        else
        {
            unsigned char c = clients->buffer[i];
            printf(" 0x%02X ", c);
        }
    }
    printf("\n\n");

    mbt_net_clients_print(clients->next);
}

bool mbt_net_clients_accept(struct mbt_net_server *server,
                            struct mbt_net_client **clients)
{
    // Accept the client
    int c_fd = accept(server->s_fd, NULL, NULL);
    if (c_fd <= 0)
    {
        return NULL;
    }

    // Register the client to the epoll instance
    struct epoll_event event = { .events = EPOLLIN | EPOLLOUT,
                                 .data = { .fd = c_fd } };
    if (epoll_ctl(server->ep_fd, EPOLL_CTL_ADD, c_fd, &event) == -1)
    {
        errx(EXIT_FAILURE, "epoll_ctl");
    }

    // Get handshake message for current context
    struct mbt_msg_handshake hs;
    mbt_msg_write_handshake(server->ctx, &hs);

    size_t sent = 0;
    void *v_hs = &hs;

    // Ensure full handshake message is sent
    // Handshake verification will be handled later
    while (sent < sizeof(struct mbt_msg_handshake))
    {
        int s = send(c_fd, v_hs, sizeof(struct mbt_msg_handshake), 0);
        if (s <= 0)
        {
            // Handle disconnection
            mbt_net_clients_remove(server, clients, c_fd);
            return false;
        }

        sent += s;
    }

    // Init new client
    struct mbt_net_client *next = xcalloc(1, sizeof(struct mbt_net_client));
    next->fd = c_fd;
    next->next = *clients;
    *clients = next;

    printf("Client accepted\n");
    return true;
}

bool mbt_net_clients_remove(struct mbt_net_server *server,
                            struct mbt_net_client **clients, int client_fd)
{
    if (!*clients)
    {
        return false;
    }

    if ((*clients)->fd != client_fd)
    {
        return mbt_net_clients_remove(server, &((*clients)->next), client_fd);
    }

    struct mbt_net_client *current = *clients;
    struct mbt_net_client *next = current->next;

    if (current->buffer)
    {
        free(current->buffer);
    }
    close(current->fd);
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
