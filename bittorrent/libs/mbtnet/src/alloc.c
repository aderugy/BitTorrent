#include <arpa/inet.h>
#include <err.h>
#include <mbt/be/torrent.h>
#include <mbt/be/torrent_getters.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/random.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <string.h>

#include "mbt/file/file_handler.h"
#include "mbt/net/msg.h"

struct mbt_net_context *mbt_net_context_init(struct mbt_torrent *t,
                                             struct in_addr ip, uint16_t port)
{
    struct mbt_net_context *ctx = xcalloc(1, sizeof(struct mbt_net_context));
    if (!ctx)
    {
        errx(EXIT_FAILURE, "Allocation error");
    }

    char *ip_buf = xcalloc(255 + 1, sizeof(char));
    inet_ntop(AF_INET, &ip, ip_buf, 255);

    char *port_str;
    asprintf(&port_str, "%d", port);

    ctx->ip = ip_buf;
    ctx->port = port_str;

    ctx->peer_id = rand_str(PEER_ID_LENGTH);
    memcpy(ctx->peer_id, MBT_PEER_ID_PREFIX, strlen(MBT_PEER_ID_PREFIX));

    ctx->announce = strdup(mbt_torrent_announce(t).data);

    ctx->info_hash = strdup(t->info->pieces->data);
    ctx->event = TRACKER_STARTED;

    ctx->left = t->info->length;

    ctx->fh = mbt_file_handler_init(t);
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
    mbt_file_handler_free(ctx->fh);

    free(ctx);
}
