#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/net/net_types.h>
#include <stdlib.h>

struct mbt_net_context *
mbt_net_context_init(__attribute((unused)) struct mbt_torrent *t,
                     struct in_addr ip, uint16_t port)
{
    struct mbt_net_context *ctx = calloc(1, sizeof(struct mbt_net_context));
    if (!ctx)
    {
        errx(EXIT_FAILURE, "Allocation error");
    }

    char *ip_buf = calloc(42 + 1, sizeof(char));
    inet_ntop(AF_INET, &ip, ip_buf, 42);

    char *port_str;
    asprintf(&port_str, "%d", port);

    ctx->torrent = t;
    ctx->ip = ip_buf;
    ctx->port = port_str;

    return ctx;
}

void mbt_net_context_free(struct mbt_net_context *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->ip)
    {
        free(ctx->ip);
    }
    if (ctx->port)
    {
        free(ctx->port);
    }

    free(ctx);
}
