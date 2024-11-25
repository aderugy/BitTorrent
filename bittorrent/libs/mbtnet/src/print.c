#include <mbt/net/net_types.h>

void mbt_net_context_print(struct mbt_net_context *ctx)
{
    printf("MBT_NET_CONTEXT\n");
    printf("\tAnnounce : %s\n", ctx->announce);
    printf("\tHost     : %s:%s\n", ctx->ip, ctx->port);

    printf("\tInfo Hash: ");
    for (size_t i = 0; i < 20; i++)
    {
        unsigned char c = ctx->info_hash[i];
        printf("%02X", c);
    }
    printf("\n");

    printf("\tPeer ID  : ");
    for (size_t i = 0; i < 20; i++)
    {
        printf("%c", ctx->peer_id[i]);
    }
    printf("\n");

    printf("\tDownloaded: %d - Uploaded: %d - Left: %zu\n", ctx->downloaded,
           ctx->uploaded, ctx->left);

    printf("END MBT_NET_CONTEXT\n\n");
}
