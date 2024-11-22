#ifndef TYPES_H
#define TYPES_H

#include <arpa/inet.h>
#include <mbt/be/torrent.h>

struct mbt_net_context
{
    struct mbt_torrent *t;
    struct in_addr ip;
    uint16_t port;
};

struct mbt_peer
{};

#endif // !TYPES_H
