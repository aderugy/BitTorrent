#ifndef NET_TYPES_H
#define NET_TYPES_H

#include <arpa/inet.h>
#include <mbt/be/torrent.h>
#include <mbt/be/types_mbtbe.h>
#include <mbt/net/context.h>
#include <mbt/net/leeching.h>
#include <mbt/net/peer.h>
#include <mbt/net/tracker.h>
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
    char *announce;

    char *ip;
    char *port;

    char *info_hash;
    char *peer_id;

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

#endif // !NET_TYPES_H
