#include <err.h>
#include <errno.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/utils/xalloc.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mbt/net/context.h"
#include "mbt/net/fifo.h"
#include "mbt/net/msg_handler.h"
#include "stdio.h"

#define MAX_CONCURRENT_IO 4

struct addrinfo *mbt_getaddrinfo(char *ip, char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *addr = NULL;
    if (getaddrinfo(ip, port, &hints, &addr))
    {
        warnx("mbt_getaddrinfo: failed");
        return NULL;
    }

    return addr;
}

struct mbt_net_server *mbt_net_server_init(struct mbt_net_context *ctx)
{
    struct mbt_net_server *server = xcalloc(1, sizeof(struct mbt_net_server));
    server->ctx = ctx;

    // Create epoll instance
    server->ep_fd = epoll_create1(EPOLL_CLOEXEC);
    if (server->ep_fd == -1)
    {
        errx(EXIT_FAILURE, "epoll_create1");
    }

    server->streams = fifo_init();
    return server;
}

void mbt_net_server_free(struct mbt_net_server *server)
{
    mbt_net_context_free(server->ctx);

    while (server->streams->size)
    {
        free(fifo_pop(server->streams));
    }
    fifo_destroy(server->streams);

    free(server);
}

static int mbt_net_server_process_out_events(struct mbt_net_server *server,
                                             struct mbt_net_client *client,
                                             struct epoll_event current)
{
    if (!(current.events & EPOLLOUT))
    {
        return MBT_HANDLER_SUCCESS;
    }

    // The client isnt connected yet
    if (client->state == MBT_CLIENT_WAITING_CONNECTION)
    {
        // Perform the connection check (waited by epoll)
        // The state is updated to MBT_HANDLER_CONNECTED
        return mbt_net_client_check_connect(client) ? MBT_HANDLER_SUCCESS
                                                    : MBT_HANDLER_CLIENT_ERROR;
    }

    // The client hasnt been hand shaken yet
    if (client->state == MBT_CLIENT_CONNECTED)
    {
        // Send the handshake to the client
        // The state is updated to MBT_CLIENT_WAITING_HANDSHAKE
        return mbt_net_client_handshake(server, client);
    }

    if (client->state == MBT_CLIENT_BITFIELD_RECEIVED)
    {
        // Set connection in interested mode
        // The state is updated to MBT_CLIENT_READY
        return mbt_msg_send_handler_interested(server, client);
    }

    return MBT_HANDLER_SUCCESS;
}

void mbt_net_server_process_event(struct mbt_net_server *server,
                                  struct mbt_net_client **clients)
{
    struct epoll_event events[MAX_CONCURRENT_IO];

    // Waiting for an I/O event
    int nb_fd = epoll_wait(server->ep_fd, events, MAX_CONCURRENT_IO, -1);
    if (nb_fd == -1)
    {
        errx(EXIT_FAILURE, "epoll_wait");
    }

    // Iterate through events
    for (int i = 0; i < nb_fd; i++)
    {
        struct epoll_event current = events[i];

        // Because we saved the client fd when we add it in the epoll
        int client_fd = current.data.fd;

        struct mbt_net_client *client =
            mbt_net_clients_find(*clients, client_fd);

        if (!client)
        {
            warnx("Unknown fd. Removing it from the peers");
            mbt_net_clients_remove(server, clients, client_fd, true);
        }

        int out_status =
            mbt_net_server_process_out_events(server, client, current);
        if (out_status != MBT_HANDLER_SUCCESS
            && out_status != MBT_HANDLER_IGNORE)
        {
            mbt_net_clients_remove(server, clients, client_fd,
                                   out_status == MBT_HANDLER_REQUEST_CLOSE);
        }

        // Make sure client has sent data to avoid blocking recv
        if ((current.events & EPOLLIN) == 0)
        {
            mbt_msg_process(server, client, NULL, 0);
            continue;
        }

        // Read client data
        char buffer[MBT_NET_BUFFER_SIZE];
        int r = recv(client->fd, buffer, MBT_NET_BUFFER_SIZE, 0);
        if (r <= 0) // Case where connection ended
        {
            if (r == -1 && errno == EINPROGRESS)
            {
                continue;
            }

            mbt_net_clients_remove(server, clients, client_fd, r == 0);
            continue;
        }
        else // Process data
        {
            mbt_msg_process(server, client, buffer, r);
        }

        mbt_net_clients_print(*clients);
    }
}
