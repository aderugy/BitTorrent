#include <curl/curl.h>
#include <curl/easy.h>
#include <err.h>
#include <mbt/net/context.h>
#include <mbt/net/types.h>
#include <stdio.h>
#include <stdlib.h>

struct mbt_peer **mbt_net_context_peers(struct mbt_net_context *ctx)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        errx(EXIT_FAILURE, "curl_easy_init");
    }

    curl_easy_cleanup(curl);
    free(ctx);
    return NULL;
}
