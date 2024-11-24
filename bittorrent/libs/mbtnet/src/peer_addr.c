#include <arpa/inet.h>
#include <err.h>
#include <mbt/net/net_types.h>
#include <stdlib.h>
#include <sys/socket.h>

void mbt_peer_addr(struct mbt_peer *peer, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = atoi(peer->port->data);

    if (!inet_pton(AF_INET, peer->ip->data, &(addr->sin_addr)))
    {
        warnx("mbt_peer_addr: inet_pton");
    }
}
