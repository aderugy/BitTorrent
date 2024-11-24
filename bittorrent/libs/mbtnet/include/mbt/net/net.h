#ifndef NET_H
#define NET_H

#include <mbt/net/net_types.h>
#include <mbt/utils/utils.h>
#include <stdbool.h>
#include <stddef.h>

#define MBT_NET_BUFFER_SIZE 1024

struct mbt_net_server
{
    int ep_fd;
    int s_fd;
    struct mbt_net_context *ctx;
};

struct mbt_net_server *mbt_net_server_init(struct mbt_net_context *ctx);

struct mbt_net_client
{
    int fd;
    bool handshaked;

    char *buffer;
    size_t read;

    struct mbt_net_client *next;
};

bool mbt_net_clients_accept(struct mbt_net_server *server,
                            struct mbt_net_client **clients) MBT_NONNULL(1, 2);

bool mbt_net_clients_remove(struct mbt_net_server *server,
                            struct mbt_net_client **clients, int client_fd)
    MBT_NONNULL(1, 2);

struct mbt_net_client *mbt_net_clients_find(struct mbt_net_client *clients,
                                            int client_fd);

#endif // !NET_H
