#include "epoll_server.h"

#include <err.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "utils/xalloc.h"

#define MAX_CONCURRENT_IO 2
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
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
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

    struct epoll_event event = { .events = EPOLLIN | EPOLLOUT,
                                 .data = { .fd = c_fd } };
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

static void process_message(struct connection_t *clients,
                            struct connection_t *client,
                            char message[BUFFER_SIZE], int read)
{
    // Allocate space for new string
    client->buffer = client->buffer == NULL
        ? xcalloc(BUFFER_SIZE, sizeof(char))
        : xrealloc(client->buffer,
                   (client->nb_read + BUFFER_SIZE) * sizeof(char));

    // Copy the new buffer
    memcpy(client->buffer + client->nb_read, message, read);
    client->nb_read += read;

    // Broadcast to clients
    if (client->buffer[client->nb_read - 1] == '\n')
    {
        while (clients)
        {
            int sent = 0;
            while (sent < client->nb_read) // Ensure everything is sent
            {
                int s = send(clients->client_socket, client->buffer + sent,
                             client->nb_read - sent, MSG_NOSIGNAL);

                if (s == -1)
                {
                    errx(EXIT_FAILURE, "send");
                }

                sent += s;
            }

            clients = clients->next;
        }

        free(client->buffer);
        client->buffer = NULL;
        client->nb_read = 0;
    }
}

static void read_events(int sfd, int epfd, struct connection_t **clients)
{
    struct epoll_event events[MAX_CONCURRENT_IO];

    // Waiting for an I/O event
    int nb_fd = epoll_wait(epfd, events, MAX_CONCURRENT_IO, -1);
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
        struct connection_t *client = NULL;

        if (client_fd == sfd)
        {
            client = accept_client(epfd, sfd, *clients);
            *clients = client;
            continue;
        }
        else
        {
            client = find_client(*clients, client_fd);
        }

        if (!client)
        {
            errx(EXIT_FAILURE, "Unknown fd");
        }

        // Read client data
        char buffer[BUFFER_SIZE];
        int r = recv(client->client_socket, buffer, BUFFER_SIZE, 0);

        if (r <= 0) // Case where connection ended
        {
            *clients = remove_client(*clients, client->client_socket);
        }
        else // Process data
        {
            process_message(*clients, client, buffer, r);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return 1;
    }

    char *ip = argv[1];
    char *port = argv[2];

    // Create epoll
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1)
    {
        return 1;
    }

    // Create, bind and listen to the socket
    int socket_fd = prepare_socket(ip, port);
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

    struct connection_t *clients = NULL;
    while (true)
    {
        // Read and process events
        read_events(socket_fd, epoll_fd, &clients);
    }

    // Close fds. Supposedly unreachable
    close(epoll_fd);
    close(socket_fd);

    // Free remaining clients
    while (clients)
    {
        int fd = clients->client_socket;
        clients = remove_client(clients, fd);
        close(fd);
    }
}
