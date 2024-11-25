#include <bits/stdint-uintn.h>
#include <mbt/file/file_types.h>
#include <mbt/net/msg.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/xalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mbt_msg_print(struct mbt_msg *msg)
{
    static char *names[] = { "MBT_MAGIC_CHOKE",      "MBT_MAGIC_UNCHOKE",
                             "MBT_MAGIC_INTERESTED", "MBT_MAGIC_NOT_INTERESTED",
                             "MBT_MAGIC_HAVE",       "MBT_MAGIC_BITFIELD",
                             "MBT_MAGIC_REQUEST",    "MBT_MAGIC_PIECE",
                             "MBT_MAGIC_CANCEL" };

    size_t length = mbt_msg_length(msg);
    printf("Received message:\n");
    printf("\tType: %s\n", names[msg->type]);
    printf("\tLength: %zu\n", length);
    printf("-- BEGIN PAYLOAD --\n");
    fwrite(msg->payload, sizeof(char), length, stdout);
    printf("\n-- END PAYLOAD --\n\n");
}

void mbt_msg_handshake_print(struct mbt_msg_handshake *msg)
{
    printf("Received handshake:\n");
    printf("\tLen: %hhi\n", msg->len);

    printf("\tProtocol: ");
    fwrite(msg->protocol, 1, MBT_HANDSHAKE_PROTOCOL_LENGTH, stdout);

    printf("\n\tInfo Hash: ");
    for (size_t i = 0; i < MBT_H_LENGTH; i++)
    {
        printf("%02X", msg->info_hash[i]);
    }

    char *peer_id = xcalloc(PEER_ID_LENGTH + 1, sizeof(char));
    memcpy(peer_id, msg->peer_id, PEER_ID_LENGTH);
    printf("\n\t Peer ID: %s\n\n", peer_id);
    free(peer_id);
}

void mbt_msg_discard(struct mbt_net_client *client, size_t size)
{
    memmove(client->buffer, client->buffer + size, client->read - size);
    client->read -= size;
}

uint64_t mbt_msg_length(struct mbt_msg *msg)
{
    void *v_buffer = msg;
    unsigned char *p_len = v_buffer;

    size_t len = 0;
    for (size_t i = 0; i < 4; i++)
    {
        len *= 10;
        len += p_len[i];
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
        && memcmp(hs.info_hash, ctx->info_hash, MBT_H_LENGTH) == 0
        && memcmp(hs.peer_id, ctx->peer_id, PEER_ID_LENGTH) == 0;
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
bool mbt_msg_process(__attribute((unused)) struct mbt_net_server *server,
                     struct mbt_net_client *client,
                     char buffer[MBT_NET_BUFFER_SIZE], int read)
{
    printf("Received message (%hhi bytes)\n", read);

    if (!client->buffer)
    {
        client->buffer = xmalloc(read * sizeof(char));
    }
    else
    {
        client->buffer =
            xrealloc(client->buffer, (client->read + read) * sizeof(char));
    }

    memcpy(client->buffer + client->read, buffer, read);
    client->read += read;

    void *v_buffer = client->buffer;
    if (!client->handshaked)
    {
        if (client->read >= sizeof(struct mbt_msg_handshake))
        {
            struct mbt_msg_handshake *hs = v_buffer;
            if (!mbt_msg_verify_handshake(server->ctx, *hs))
            {
                printf("Handshake failed\n");
                return false;
            }

            mbt_msg_handshake_print(hs);

            client->handshaked = true;
            mbt_msg_discard(client, sizeof(struct mbt_msg_handshake));
        }
    }
    else
    {
        struct mbt_msg *msg = v_buffer;
        if (client->read < 4 || client->read < mbt_msg_length(msg))
        {
            return true;
        }

        mbt_msg_print(msg);
        mbt_msg_discard(client, sizeof(struct mbt_msg) + mbt_msg_length(msg));
    }

    return true;
}
