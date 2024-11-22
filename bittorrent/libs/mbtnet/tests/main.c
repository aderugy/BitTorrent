#include <mbt/net/context.h>
#include <mbt/net/net_types.h>
#include <mbt/net/peer.h>
#include <stdlib.h>

int main(void)
{
    struct mbt_net_context *ctx = calloc(1, sizeof(struct mbt_net_context));
    ctx->ip = "google.com";
    ctx->port = "80";

    mbt_net_context_peers(ctx);
    return 0;
}
