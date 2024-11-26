#include <err.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>
#include <mbt/utils/xalloc.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mbt/file/file_handler.h"
#include "mbt/net/context.h"
#include "mbt/net/msg_handler.h"

#define MAX_CONCURRENT_IO 4

/**
 * \brief Iterate over the struct addrinfo elements to create and bind a socket
 *
 * \param addrinfo: struct addrinfo elements
 *
 * \return The created socket or exit with 1 if there is an error
 *
 * Try to create and connect a socket with every addrinfo element until it
 * succeeds
 *
 */
static int create_and_bind(struct addrinfo *addrinfo)
{
    while (addrinfo)
    {
        int s_fd = socket(addrinfo->ai_family, addrinfo->ai_socktype,
                          addrinfo->ai_protocol);
        if (s_fd > 0)
        {
            if (!bind(s_fd, addrinfo->ai_addr, addrinfo->ai_addrlen))
            {
                return s_fd;
            }

            close(s_fd);
        }

        addrinfo = addrinfo->ai_next;
    }

    errx(EXIT_FAILURE, "create_and_bind");
}

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

/**
 * \brief Initialize the Addrinfo struct and call create_and bind() and
 * listen(2)
 *
 * \param ip: IP address of the server
 * \param port: Port of the server
 *
 * \return The created socket
 *
 * Initialize the struct addrinfo needed by create_and_bind() before calling
 * it. When create_and_bind() returns a valid socket, set the socket to
 * listening and return it.
 */
static int prepare_socket(char *ip, char *port, struct mbt_net_server *server)
{
    if ((server->addr = mbt_getaddrinfo(ip, port)) == NULL)
    {
        errx(1, "getaddrinfo");
    }

    int s_fd = create_and_bind(server->addr);
    if (s_fd <= 0)
    {
        errx(1, "prepare_socket");
    }

    if (listen(s_fd, 10))
    {
        errx(1, "listen");
    }

    printf("Listening on %s:%s\n", ip, port);
    return s_fd;
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

    // Create, bind and listen to the socket
    server->s_fd = prepare_socket(ctx->ip, ctx->port, server);
    if (server->s_fd <= 0)
    {
        errx(EXIT_FAILURE, "prepare_socket");
    }

    // We add the socket to the list of interests
    struct epoll_event event = { .events = EPOLLIN | EPOLLOUT,
                                 .data = { .fd = server->s_fd } };

    if (epoll_ctl(server->ep_fd, EPOLL_CTL_ADD, server->s_fd, &event) == -1)
    {
        errx(EXIT_FAILURE, "create epoll");
    }

    return server;
}

void mbt_net_server_free(struct mbt_net_server *server)
{
    freeaddrinfo(server->addr);
    mbt_net_context_free(server->ctx);
    free(server);
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
        if (client_fd == server->s_fd)
        {
            warnx("Attempt to connect.\n");
            continue;
        }

        struct mbt_net_client *client =
            mbt_net_clients_find(*clients, client_fd);
        if (!client)
        {
            errx(EXIT_FAILURE, "Unknown fd");
        }

        // The client isnt connected yet
        if (client->state == MBT_CLIENT_WAITING_CONNECTION
            && current.events | EPOLLOUT)
        {
            // Perform the connection check (waited by epoll)
            // The state is updated to MBT_HANDLER_CONNECTED
            if (!mbt_net_client_check_connect(client))
            {
                mbt_net_clients_remove(server, clients, client_fd, false);
            }
        }

        // The client hasnt been hand shaken yet
        if (client->state == MBT_CLIENT_CONNECTED && current.events | EPOLLOUT)
        {
            // Send the handshake to the client
            // The state is updated to MBT_HANDLER_WAITING_HANDSHAKE
            int status = mbt_net_client_handshake(server, client);
            if (status != MBT_HANDLER_SUCCESS)
            {
                mbt_net_clients_remove(server, clients, client_fd,
                                       status == MBT_HANDLER_REQUEST_CLOSE);
            }
        }

        if (client->state == MBT_CLIENT_BITFIELD_RECEIVED

            && current.events | EPOLLOUT)
        {
            int status = mbt_msg_send_handler_interested(server, client);
            if (status != MBT_HANDLER_SUCCESS)
            {
                mbt_net_clients_remove(server, clients, client_fd,
                                       status == MBT_HANDLER_REQUEST_CLOSE);
            }
        }

        if (client->state == MBT_CLIENT_REQUESTING && !client->choked)
        {
            int status = mbt_msg_send_handler_request(server, client);
            if (status != MBT_HANDLER_SUCCESS)
            {
                mbt_net_clients_remove(server, clients, client_fd,
                                       status == MBT_HANDLER_REQUEST_CLOSE);
            }
        }

        // Make sure client has sent data to avoid blocking recv
        if ((current.events & EPOLLIN) == 0)
        {
            continue;
        }

        // Read client data
        char buffer[MBT_NET_BUFFER_SIZE];
        int r = recv(client->fd, buffer, MBT_NET_BUFFER_SIZE, 0);
        if (r <= 0) // Case where connection ended
        {
            mbt_net_clients_remove(server, clients, client_fd, r == 0);
        }
        else // Process data
        {
            mbt_msg_process(server, client, buffer, r);
        }

        mbt_net_clients_print(*clients);
    }
}
