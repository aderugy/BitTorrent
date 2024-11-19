#include "epoll_server.h"

#include <netdb.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "utils/xalloc.h"

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

    return 0;
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

    char *hostname;
    asprintf(&hostname, "%s:%s", ip, port);

    if (getaddrinfo(hostname, NULL, NULL, &addrinfo))
    {
        return -1;
    }

    int s_fd = create_and_bind(addrinfo);
    freeaddrinfo(addrinfo);

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

    struct epoll_event *event = xcalloc(1, sizeof(struct epoll_event));
    event->events = EPOLLIN | EPOLLOUT;
    epoll_ctl(epoll_instance, EPOLL_CTL_ADD, c_fd, event);

    struct connection_t *next = add_client(connection, c_fd);
    if (!next)
    {
        epoll_ctl(epoll_instance, EPOLL_CTL_DEL, c_fd, NULL);
    }

    return next;
}
