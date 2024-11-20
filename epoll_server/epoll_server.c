#include "epoll_server.h"

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "utils/xalloc.h"

#define MAX_CONCURRENT_IO 10
#define BUFFER_SIZE 1000

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
int create_and_bind(struct addrinfo *addrinfo)
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
int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo *addrinfo = NULL;
    if (getaddrinfo(ip, port, NULL, &addrinfo))
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

/**
 * \brief Accept a new client and add it to the connection_t struct
 *
 * \param epoll_instance: the epoll instance
 * \param server_socket: listening socket
 * \param connection: the connection linked list with all the current
 * connections
 *
 * \return The connection struct with the new client added
 */
struct connection_t *accept_client(int epoll_instance, int server_socket,
                                   struct connection_t *connection)
{
    int c_fd = accept(server_socket, NULL, NULL);
    if (c_fd <= 0)
    {
        return NULL;
    }

    struct epoll_event event = { .events = EPOLLIN, .data = { .fd = c_fd } };
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, c_fd, &event) == -1)
    {
        errx(EXIT_FAILURE, "epoll_ctl");
    }

    struct connection_t *next = add_client(connection, c_fd);
    if (!next)
    {
        errx(EXIT_FAILURE, "add_client");
    }

    return next;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return 1;
    }

    char *ip = argv[1];
    char *port = argv[2];

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        return 1;
    }

    int socket_fd = prepare_socket(ip, port);
    if (socket_fd <= 0)
    {
        errx(EXIT_FAILURE, "prepare_socket");
    }

    struct epoll_event event = { .events = EPOLLIN,
                                 .data = { .fd = socket_fd } };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
    {
        errx(EXIT_FAILURE, "create epoll");
    }

    struct connection_t *clients = NULL;
    while (42)
    {
        struct epoll_event events[MAX_CONCURRENT_IO];

        int nb_fd = epoll_wait(epoll_fd, events, MAX_CONCURRENT_IO, -1);
        if (nb_fd == -1)
        {
            errx(EXIT_FAILURE, "epoll_wait");
        }

        for (int i = 0; i < nb_fd; i++)
        {
            struct epoll_event current = events[i];
            int client_fd = current.data.fd;

            struct connection_t *client = find_client(clients, client_fd);
            if (!client)
            {
                client = accept_client(epoll_fd, socket_fd, clients);
                clients = client;

                if (!client)
                {
                    errx(EXIT_FAILURE, "Unknown fd");
                }
            }

            char buffer[BUFFER_SIZE + 1];
            int r = recv(client->client_socket, buffer, BUFFER_SIZE, 0);

            if (r <= 0)
            {
                printf("Removing client\n");
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->client_socket, NULL);
                clients = remove_client(clients, client->client_socket);
            }
            else
            {
                buffer[r] = 0;
                printf("%s", buffer);
            }
        }
    }

    close(epoll_fd);
    close(socket_fd);

    while (clients)
    {
        int fd = clients->client_socket;
        clients = remove_client(clients, fd);
        close(fd);
    }
}
