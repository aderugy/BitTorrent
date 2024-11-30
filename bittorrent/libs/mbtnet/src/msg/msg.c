#include <bits/stdint-uintn.h>
#include <err.h>
#include <mbt/file/file_types.h>
#include <mbt/net/msg.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/logger.h>
#include <mbt/utils/xalloc.h>
#include <stdio.h>
#include <string.h>

#include "mbt/net/msg_handler.h"
#include "mbt/net/net.h"

void mbt_msg_print(struct mbt_msg *msg)
{
    static char *names[] = { "CHOKE",          "UNCHOKE", "INTERESTED",
                             "NOT_INTERESTED", "HAVE",    "BITFIELD",
                             "REQUEST",        "PIECE",   "CANCEL" };

    size_t length = mbt_msg_length(msg);
    if (length == 0)
    {
        logger("recv: keepalive\n");
        return;
    }

    logger("recv: %s: %zu bytes\n", names[msg->type], length);
}

void mbt_msg_handshake_print(struct mbt_msg_handshake *msg)
{
    logger_buffer("recv: handshake", msg->peer_id, 20);
}

void mbt_msg_discard(struct mbt_net_client *client, size_t size)
{
    memmove(client->buffer, client->buffer + size, client->read - size);
    client->read -= size;
}

uint32_t mbt_msg_length(struct mbt_msg *msg)
{
    void *v_buffer = msg;
    unsigned char *buf = v_buffer;

    uint32_t len = 0;
    for (size_t i = 0; i < 4; i++)
    {
        len <<= 8;
        len += buf[i];
    }

    return len;
}

// Verifies the handshake received in hs struct against ctx
bool mbt_msg_verify_handshake(struct mbt_net_context *ctx,
                              struct mbt_msg_handshake hs)
{
    return hs.len == MBT_HANDSHAKE_PROTOCOL_LENGTH
        && memcmp(hs.protocol, MBT_HANDSHAKE_PROTOCOL,
                  MBT_HANDSHAKE_PROTOCOL_LENGTH)
        == 0
        && memcmp(hs.info_hash, ctx->info_hash, MBT_H_LENGTH) == 0;
}

// Writes the handshake payload to struct hs
void mbt_msg_write_handshake(struct mbt_net_context *ctx,
                             struct mbt_msg_handshake *hs)
{
    hs->len = MBT_HANDSHAKE_PROTOCOL_LENGTH;
    memcpy(hs->protocol, MBT_HANDSHAKE_PROTOCOL, MBT_HANDSHAKE_PROTOCOL_LENGTH);
    memset(hs->extension, 0, 8);
    memcpy(hs->info_hash, ctx->info_hash, MBT_H_LENGTH);
    memcpy(hs->peer_id, ctx->peer_id, PEER_ID_LENGTH);
}

/*
 * Algorithm :
 * 1/ Allocate buffer memory for the message
 * 2/ Copy the message after existing data
 * 3/ Checks the connection status with the client to appropriately process msg
 *      3.1/ If expecting the handshake: verify it against the server's
 *      3.2/ If the message is incomplete, nothing done
 *      3.3/ If the message is complete, process it and discard it
 *      3.3/ On EOF: remove client (TODO)
 *      3.4/ On error, remove client (TODO)
 */
bool mbt_msg_process(struct mbt_net_server *server,
                     struct mbt_net_client *client,
                     char buffer[MBT_NET_BUFFER_SIZE], int read)
{
    if (buffer)
    {
        if (!client->buffer)
        {
            client->buffer = xcalloc(read, sizeof(char));
        }
        else
        {
            client->buffer =
                xrealloc(client->buffer, (client->read + read) * sizeof(char));
        }

        memcpy(client->buffer + client->read, buffer, read);
        client->read += read;
    }

    void *v_buffer = client->buffer;
    if (client->state == MBT_CLIENT_WAITING_HANDSHAKE)
    {
        if (client->read >= sizeof(struct mbt_msg_handshake))
        {
            struct mbt_msg_handshake *hs = v_buffer;
            if (!mbt_msg_verify_handshake(server->ctx, *hs))
            {
                warnx("mbt_msg_verify_handshake: failed");
                return false;
            }

            mbt_msg_handshake_print(hs);

            client->state = MBT_CLIENT_HANDSHAKEN;
            mbt_msg_discard(client, sizeof(struct mbt_msg_handshake));
        }
    }
    else
    {
        struct mbt_msg *msg = v_buffer;
        if (client->read < 4 || client->read < mbt_msg_length(msg) + 4)
        {
            return true;
        }

        mbt_msg_print(msg);
        if (mbt_msg_handler(server, client, msg) != MBT_HANDLER_IGNORE)
        {
            mbt_msg_discard(client, 4 + mbt_msg_length(msg));
        }
    }

    return true;
}
