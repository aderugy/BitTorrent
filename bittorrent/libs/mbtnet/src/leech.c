#include <mbt/net/net_types.h>

#include "mbt/net/msg.h"
#include "mbt/net/net.h"

void mbt_leech(struct mbt_net_context *ctx)
{
    if (ctx->left == 0)
    {
        return;
    }

    struct mbt_net_server *server = mbt_net_server_init(ctx);
    struct mbt_net_client *clients = NULL;

    while (true)
    {
        mbt_net_server_process_event(server, &clients);
    }
}
