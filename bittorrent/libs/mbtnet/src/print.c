#include <mbt/net/net_types.h>
#include <mbt/utils/logger.h>

void mbt_net_context_print(struct mbt_net_context *ctx)
{
    logger("MBT_NET_CONTEXT\n");
    logger("\tAnnounce : %s\n", ctx->announce);
    logger("\tHost     : %s:%s\n", ctx->ip, ctx->port);

    logger("\tInfo Hash: ");
    for (size_t i = 0; i < 20; i++)
    {
        unsigned char c = ctx->info_hash[i];
        logger("%02X", c);
    }
    logger("\n");

    logger("\tPeer ID  : ");
    for (size_t i = 0; i < 20; i++)
    {
        logger("%c", ctx->peer_id[i]);
    }
    logger("\n");

    logger("\tDownloaded: %d - Uploaded: %d - Left: %zu\n", ctx->downloaded,
           ctx->uploaded, ctx->left);
    logger("\tFile Handler: %p\n", ctx->fh);

    logger("END MBT_NET_CONTEXT\n\n");
}
