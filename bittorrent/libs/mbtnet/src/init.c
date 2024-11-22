#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/net/context.h>
#include <mbt/net/types.h>
#include <stdlib.h>

struct mbt_net_context *mbt_net_context_init(struct mbt_torrent *t,
                                             struct in_addr ip, uint16_t port)
{
    struct mbt_net_context *ctx = calloc(1, sizeof(struct mbt_net_context));
    if (!ctx)
    {
        errx(EXIT_FAILURE, "Allocation error");
    }

    ctx->t = t;
    ctx->ip = ip;
    ctx->port = port;

    return ctx;
}

void mbt_net_context_free(struct mbt_net_context *ctx)
{
    if (!ctx)
    {
        return;
    }

    mbt_torrent_free(ctx->t);
    free(ctx);
}
