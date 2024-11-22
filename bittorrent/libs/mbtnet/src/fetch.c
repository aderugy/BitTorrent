#include <curl/curl.h>
#include <curl/easy.h>
#include <err.h>
#include <mbt/net/net_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct curl_data
{
    char *response;
    size_t size;
};

// Write callback of curl.
static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    struct curl_data *mem = (struct curl_data *)clientp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr)
        return 0; /* out of curl_data! */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    printf("%s\n", mem->response);
    return realsize;
}

static char *add_iparam(__attribute((unused)) CURL *curl, char *src,
                        const char *name, int value, bool is_first)
{
    char *out = NULL;

    asprintf(&out, "%s%s%s=%d", src, is_first ? "?" : "&", name, value);
    free(src);

    return out;
}

static char *add_param(CURL *curl, char *src, const char *name, char *value,
                       bool is_first)
{
    if (!value)
    {
        return src;
    }

    char *out;
    // URL Encoding of param value
    char *encoded_value = curl_easy_escape(curl, value, strlen(value));

    asprintf(&out, "%s%s%s=%s", src, is_first ? "?" : "&", name, encoded_value);
    free(encoded_value);
    free(src);

    return out;
}

static char *get_event_value(enum tracker_event event)
{
    switch (event)
    {
    case TRACKER_EMPTY:
        return "empty";
    case TRACKER_STARTED:
        return "started";
    case TRACKER_COMPLETED:
        return "completed";
    case TRACKER_STOPPED:
        return "stopped";
    }

    errx(EXIT_FAILURE, "get_event_value: no such event");
}

static char *get_tracker_url(CURL *curl, struct mbt_net_context *ctx)
{
    char *params = calloc(1, sizeof(char)); // Search params
    params = add_iparam(curl, params, "left", ctx->left, true);
    params =
        add_param(curl, params, "event", get_event_value(ctx->event), false);
    params = add_iparam(curl, params, "uploaded", ctx->uploaded, false);
    params = add_iparam(curl, params, "downloaded", ctx->downloaded, false);

    params = add_param(curl, params, "ip", ctx->ip, false);
    params = add_param(curl, params, "port", ctx->port, false);
    params = add_param(curl, params, "peer_id", ctx->peer_id, false);
    params = add_param(curl, params, "info_hash", ctx->info_hash, false);

    char *url; // Full URL
    asprintf(&url, "%s%s", ctx->announce, params);

    free(params);
    return url;
}

struct mbt_peer **mbt_net_context_peers(struct mbt_net_context *ctx)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        errx(EXIT_FAILURE, "curl_easy_init");
    }

    // Output struct
    struct curl_data chunk;
    memset(&chunk, 0, sizeof(struct curl_data));

    // Format URL
    char *encoded_url = get_tracker_url(curl, ctx);

    // Curl Options
    curl_easy_setopt(curl, CURLOPT_URL, encoded_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl); // Sending request
    curl_easy_cleanup(curl); // Cleaning
    free(encoded_url);

    if (res != CURLE_OK)
    {
        warnx("Curl failed: %s", curl_easy_strerror(res));
        return NULL;
    }

    return NULL;
}
