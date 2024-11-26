#ifndef NET_H
#define NET_H

#include <mbt/file/file_types.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/utils.h>
#include <stdbool.h>
#include <stddef.h>

#define MBT_NET_BUFFER_SIZE 1024

enum mbt_client_state
{
    MBT_CLIENT_WAITING_CONNECTION = 0, // Waiting for connect
    MBT_CLIENT_CONNECTED, // Waiting for handshake
    MBT_CLIENT_WAITING_HANDSHAKE,
    MBT_CLIENT_HANDSHAKEN, // Waiting for bitfield
    MBT_CLIENT_BITFIELD_RECEIVED, // Waiting for unchoked
    MBT_CLIENT_REQUESTING, // Waiting for first block
    MBT_CLIENT_DOWNLOADED, // Waiting for full piece
    MBT_CLIENT_COMPLETED, // Back to MBT_CLIENT_REQUESTING
    MBT_CLIENT_ERROR, // SEGPIPE
    MBT_CLIENT_DISCONNECTED // CLEAN EXIT
};

struct mbt_net_request
{
    uint32_t index;
    uint32_t begin;
    uint32_t length;
};

struct mbt_net_server
{
    int ep_fd;
    int s_fd;

    struct addrinfo *addr;
    struct mbt_net_context *ctx;
};

struct mbt_net_client
{
    int fd;

    char *buffer;
    size_t read;

    bool choked;
    bool *bitfield;
    enum mbt_client_state state;

    struct mbt_net_request request;
    struct mbt_net_client *next;
};

// SERVER FUNCTIONS
struct addrinfo *mbt_getaddrinfo(char *ip, char *port);

struct mbt_net_server *mbt_net_server_init(struct mbt_net_context *ctx);
void mbt_net_server_free(struct mbt_net_server *server);
// SERVER FUNCTIONS

// CLIENT FUNCTIONS
int mbt_net_client_handshake(struct mbt_net_server *server,
                             struct mbt_net_client *client) MBT_NONNULL(1, 2);

void mbt_net_clients_print(struct mbt_net_client *clients);

bool mbt_net_clients_add(struct mbt_net_server *server,
                         struct mbt_net_client **clients, int c_fd,
                         enum mbt_client_state state) MBT_NONNULL(1, 2);

bool mbt_net_peer_connect(struct mbt_net_server *server,
                          struct mbt_net_client **clients,
                          struct mbt_peer *peer) MBT_NONNULL(1, 2, 3);

bool mbt_net_client_check_connect(struct mbt_net_client *client) MBT_NONNULL(1);

bool mbt_net_clients_remove(struct mbt_net_server *server,
                            struct mbt_net_client **clients, int client_fd,
                            bool close_fd) MBT_NONNULL(1, 2);

struct mbt_net_client *mbt_net_clients_find(struct mbt_net_client *clients,
                                            int client_fd);
// CLIENT FUNCTIONS

#endif // !NET_H
