#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/torrent_getters.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/random.h>
#include <stdlib.h>
#include <string.h>

// @TODO: track the file status
struct mbt_net_context *mbt_net_context_init(struct mbt_torrent *t,
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

    ctx->ip = ip_buf;
    ctx->port = port_str;

    ctx->peer_id = rand_str(PEER_ID_LENGTH);
    ctx->announce = strdup(mbt_torrent_announce(t).data);
    ctx->info_hash = strdup(mbt_torrent_pieces(t).data);
    ctx->event = TRACKER_STARTED;

    return ctx;
}

void mbt_net_context_free(struct mbt_net_context *ctx)
{
    if (!ctx)
    {
        return;
    }

    free(ctx->ip);
    free(ctx->port);
    free(ctx->info_hash);
    free(ctx->announce);
    free(ctx->peer_id);

    free(ctx);
}
