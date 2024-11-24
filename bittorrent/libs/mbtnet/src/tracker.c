#include <mbt/net/net_types.h>
#include <stdlib.h>

int mbt_net_contact_tracker(struct mbt_net_context *ctx)
{
    struct mbt_peer **peers = mbt_net_context_peers(ctx);
    if (!peers)
    {
        return -1;
    }

    struct mbt_peer **tmp = peers;

    size_t len = 0;
    while (*peers)
    {
        mbt_peer_free(*peers);

        len++;
        (*peers)++;
    }

    free(tmp);
    return len;
}
