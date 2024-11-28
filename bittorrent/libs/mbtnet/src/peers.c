#include <curl/curl.h>
#include <curl/easy.h>
#include <err.h>
#include <mbt/be/bencode.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/str.h>
#include <mbt/utils/utils.h>
#include <mbt/utils/view.h>
#include <mbt/utils/xalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arpa/inet.h"
#include "bits/stdint-uintn.h"
#include "mbt/net/net.h"

struct curl_data
{
    char *response;
    size_t size;
};

// Write callback of curl.
static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    struct curl_data *mem = clientp;

    char *ptr = xrealloc(mem->response, mem->size + realsize + 1);

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

// Adds an int URL Search param to the given url. Frees the old one
static char *add_iparam(__attribute((unused)) CURL *curl, char *src,
                        const char *name, int value, bool is_first)
{
    char *out = NULL;

    asprintf(&out, "%s%s%s=%d", src, is_first ? "?" : "&", name, value);
    free(src);

    return out;
}

// Adds a str URL Search param to the given url. Frees the old one
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
        return "";
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
    params = add_iparam(curl, params, "no_peer_id", 1, false);

    params = add_param(curl, params, "ip", ctx->ip, false);
    params = add_param(curl, params, "port", ctx->port, false);
    params = add_param(curl, params, "peer_id", ctx->peer_id, false);
    params = add_param(curl, params, "info_hash", ctx->info_hash, false);

    char *url; // Full URL
    asprintf(&url, "%s%s", ctx->announce, params);

    free(params);
    return url;
}

static void print_peer(struct mbt_peer *peer)
{
    printf("PEER (%s);\n\tip: %s\n\tport: %s\n\n", peer->id->data,
           peer->ip->data, peer->port->data);
}

static struct mbt_peer *bdecode_peer(struct mbt_be_pair **bpeer)
{
    struct mbt_peer *peer = xcalloc(1, sizeof(struct mbt_peer));

    for (struct mbt_be_pair *node = *bpeer; node; node = *(++bpeer))
    {
        /*
         * TYPE
         * {
         *      "peer id": str,
         *      "ip": str,
         *      "port": nb
         * }
         */
        struct mbt_cview key = MBT_CVIEW_OF(node->key);
        struct mbt_be_node *value = node->val;

        if (strcmp(key.data, "peer id") == 0)
        {
            struct mbt_cview cpid = MBT_CVIEW_OF(value->v.str);
            peer->id = mbt_str_init(cpid.size);
            mbt_str_pushcv(peer->id, cpid);
        }
        else if (strcmp(key.data, "ip") == 0)
        {
            struct mbt_cview cpip = MBT_CVIEW_OF(value->v.str);
            peer->ip = mbt_str_init(cpip.size);
            mbt_str_pushcv(peer->ip, cpip);
        }
        else if (strcmp(key.data, "port") == 0)
        {
            peer->port = mbt_str_init(6);

            char *str_port = NULL;
            asprintf(&str_port, "%zu", value->v.nb);
            mbt_str_pushcstr(peer->port, str_port);
            free(str_port);
        }
        else
        {
            warnx("Unknown key: '%s'", key.data);
            free(peer);
            return NULL;
        }
    }

    print_peer(peer);
    return peer;
}

static struct mbt_peer **parse_peers_list(struct mbt_be_node **in)
{
    // Count peers
    size_t nb_peers = 0;
    for (; in[nb_peers]; nb_peers++)
    {
        continue;
    }

    struct mbt_peer **peers = xcalloc(nb_peers + 1, sizeof(struct mbt_peer *));
    for (size_t i = 0; i < nb_peers; i++)
    {
        struct mbt_peer *peer;
        if ((peer = bdecode_peer(in[i]->v.dict)) == NULL)
        {
            for (size_t j = 0; peers[j]; j++)
            {
                mbt_peer_free(peers[j]);
            }
            free(peers);

            warnx("parse_peers_list: failed to parse peer");
            return NULL;
        }

        peers[i] = peer;
    }

    return peers;
}

static struct mbt_peer **parse_peers_list_compact(struct mbt_str str)
{
    /*
     * Format (big endian):
     * IPv4 32 bit     Port
     * AA BB CC DD    FF FF (and so forth)
     */
    size_t nb_peers = str.size / 6;
    struct mbt_peer **peers = xcalloc(nb_peers + 1, sizeof(struct mbt_peer *));

    char *ptr = str.data;
    for (size_t i = 0; i < nb_peers; i++)
    {
        struct mbt_peer *peer = xcalloc(1, sizeof(struct mbt_peer));
        peer->id = mbt_str_init(PEER_ID_LENGTH);
        peer->ip = mbt_str_init(255);
        peer->port = mbt_str_init(10);

        inet_ntop(AF_INET, ptr, peer->ip->data, PEER_ID_LENGTH);

        char *port_str;
        int port = (ptr[4] << 8) + ptr[5]; // Big Endian

        asprintf(&port_str, "%d", port);
        mbt_str_pushcstr(peer->port, port_str);
        free(port_str);

        print_peer(peer);
        peers[i] = peer;

        ptr += 6;
    }

    return peers;
}

static struct mbt_peer **parse_output(struct curl_data data)
{
    struct mbt_cview buf = { .data = data.response, .size = data.size };

    /*
     * TYPE (NOT COMPACT !!)
     * {
     *   "interval": number,
     *   "peers": [
     *      {
     *          "peer id": str,
     *          "ip": str,
     *          "port": nb
     *      }
     *   ],
     *   "failure": str
     * }
     */
    struct mbt_be_node *root = mbt_be_decode(&buf);
    if (!root || root->type != MBT_BE_DICT || !root->v.dict)
    {
        errx(EXIT_FAILURE, "parse_output: invalid data");
    }

    uint64_t interval = 0;
    struct mbt_peer **peers = NULL;

    struct mbt_be_pair **dict = root->v.dict; // Root is a dict
    for (struct mbt_be_pair *node = *dict; node; node = *(++dict))
    {
        struct mbt_cview key = MBT_CVIEW_OF(node->key);
        struct mbt_be_node *value = node->val;

        if (strcmp(key.data, "interval") == 0 && value->type == MBT_BE_NUM)
        {
            interval = value->v.nb;
        }
        else if (strcmp(key.data, "peers") == 0
                 && (value->type == MBT_BE_LIST || value->type == MBT_BE_STR))
        {
            peers = value->type == MBT_BE_STR
                ? parse_peers_list_compact(value->v.str)
                : parse_peers_list(value->v.list);
        }
        else // Key incorrect or type is wrong
        {
            if (strcmp(key.data, "failure reason") == 0
                && value->type == MBT_BE_STR)
            {
                warnx("%s", value->v.str.data);
            }
            else
            {
                warnx("Invalid data (key: '%s')", key.data);
            }
        }
    }

    if (!peers || interval == 0)
    {
        errx(EXIT_FAILURE, "Incomplete data (peer: %p - interval: %zu)", peers,
             interval);
    }

    mbt_be_free(root);
    free(data.response);
    return peers;
}

void mbt_peer_init_addr(struct mbt_peer *peer)
{
    peer->addr = mbt_getaddrinfo(peer->ip->data, peer->port->data);
}

void mbt_peer_addr(struct mbt_peer *peer, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = atoi(peer->port->data);

    if (!inet_pton(AF_INET, peer->ip->data, &(addr->sin_addr)))
    {
        warnx("mbt_peer_addr: inet_pton");
    }
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    printf("GET %s\n", encoded_url);
    CURLcode res = curl_easy_perform(curl); // Sending request
    curl_easy_cleanup(curl); // Cleaning
    free(encoded_url);

    if (res != CURLE_OK)
    {
        errx(EXIT_FAILURE, "Curl failed: %s", curl_easy_strerror(res));
    }

    return parse_output(chunk);
}

void mbt_peer_free(struct mbt_peer *peer)
{
    if (!peer)
    {
        return;
    }

    mbt_str_free(peer->id);
    mbt_str_free(peer->ip);
    mbt_str_free(peer->port);
    free(peer);
}
