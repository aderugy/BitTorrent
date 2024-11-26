#include <mbt/net/net_types.h>
#include <stdlib.h>

int mbt_net_contact_tracker(struct mbt_net_context *ctx)
{
    struct mbt_peer **peers = mbt_net_context_peers(ctx);
    if (!peers)
    {
        return -1;
    }

    size_t len = 0;
    for (; peers[len]; len++)
    {
        mbt_peer_free(peers[len]);
    }
    free(peers);

    return len;
}
