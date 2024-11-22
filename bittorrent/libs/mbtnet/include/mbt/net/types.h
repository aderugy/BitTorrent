#ifndef TYPES_H
#define TYPES_H

#include <arpa/inet.h>
#include <mbt/be/torrent.h>
#include <mbt/utils/str.h>

#include "bits/stdint-uintn.h"

enum tracker_event
{
    TRACKER_EMPTY = 0,
    TRACKER_STARTED,
    TRACKER_COMPLETED,
    TRACKER_STOPPED
};

struct mbt_net_context
{
    struct mbt_torrent *t;
    struct in_addr ip;
    uint16_t port;

    uint16_t downloaded;
    uint16_t uploaded;

    uint64_t left;
    enum tracker_event event;
};

struct mbt_peer
{
    struct mbt_str *id;
    struct mbt_str *ip;
    struct mbt_str *port;
};

#endif // !TYPES_H
