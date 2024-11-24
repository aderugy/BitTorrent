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
static int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *addrinfo = NULL;
    if (getaddrinfo(ip, port, &hints, &addrinfo))
    {
        errx(1, "getaddrinfo");
    }

    int s_fd = create_and_bind(addrinfo);
    freeaddrinfo(addrinfo);

    if (s_fd <= 0)
    {
        errx(1, "prepare_socket");
    }

    if (listen(s_fd, 10))
    {
        errx(1, "listen");
    }

    return s_fd;
}

struct mbt_net_server *mbt_net_server_init(struct mbt_net_context *ctx)
{
    // Create epoll instance
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1)
    {
        errx(EXIT_FAILURE, "epoll_create1");
    }

    // Create, bind and listen to the socket
    int socket_fd = prepare_socket(ctx->ip, ctx->port);
    if (socket_fd <= 0)
    {
        errx(EXIT_FAILURE, "prepare_socket");
    }

    // We add the socket to the list of interests
    struct epoll_event event = { .events = EPOLLIN | EPOLLOUT,
                                 .data = { .fd = socket_fd } };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
    {
        errx(EXIT_FAILURE, "create epoll");
    }

    struct mbt_net_server *server = xcalloc(1, sizeof(struct mbt_net_server));
    server->s_fd = socket_fd;
    server->ep_fd = epoll_fd;
    server->ctx = ctx;

    return server;
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

        // Make sure client has sent data to avoid blocking recv
        if ((current.events & EPOLLIN) == 0)
        {
            continue;
        }

        // Because we saved the client fd when we add it in the epoll
        int client_fd = current.data.fd;
        struct mbt_net_client *client = NULL;

        if (client_fd == server->s_fd)
        {
            if (!mbt_net_clients_accept(server, clients))
            {
                warnx("mbt_net_clients_accept: failed to accept client");
            }
            continue;
        }
        else
        {
            client = mbt_net_clients_find(*clients, client_fd);
        }

        if (!client)
        {
            errx(EXIT_FAILURE, "Unknown fd");
        }

        // Read client data
        char buffer[MBT_NET_BUFFER_SIZE];
        int r = recv(client->fd, buffer, MBT_NET_BUFFER_SIZE, 0);

        if (r <= 0) // Case where connection ended
        {
            mbt_net_clients_remove(server, clients, client_fd);
        }
        else // Process data
        {
            mbt_msg_process(server, client, buffer, r);
        }
    }
}
