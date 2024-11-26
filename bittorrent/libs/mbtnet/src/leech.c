#include <err.h>
#include <mbt/net/net_types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "mbt/net/context.h"
#include "mbt/net/msg.h"
#include "mbt/net/net.h"
#include "mbt/net/peer.h"
#include "stdlib.h"
#include "string.h"

/*
 * Algorithm :
 * 1/ Get list of peers
 * 2/ Connect to each peer
 *      2.1/ Create a client socket
 *      2.2/ Mark it as non blocked
 *      2.3/ Add it to the EPOLL
 *  3/ Wait for events
 *      3.1/ Connect event:
 *          3.1.1/ Successful
 *              3.1.1.1/
 */
void mbt_leech(struct mbt_net_context *ctx)
{
    if (ctx->left == 0)
    {
        return;
    }

    // Get list of peers to contact
    struct mbt_peer **peers = mbt_net_context_peers(ctx);
    if (!peers)
    {
        errx(EXIT_FAILURE, "mbt_leech: peers or server");
    }

    struct mbt_net_client *clients = NULL;
    struct mbt_net_server *server = mbt_net_server_init(ctx);

    for (size_t i = 0; peers[i]; i++)
    {
        struct mbt_peer *peer = peers[i];
        mbt_net_peer_connect(server, &clients, peer);
    }

    for (size_t i = 0; peers[i]; i++)
    {
        free(peers[i]);
    }
    free(peers);

    while (true)
    {
        mbt_net_server_process_event(server, &clients);
    }
}
