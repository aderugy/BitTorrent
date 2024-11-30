#include <bits/stdint-uintn.h>
#include <err.h>
#include <mbt/be/bencode.h>
#include <mbt/file/file_handler.h>
#include <mbt/file/piece.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/utils/str.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <string.h>

#include "mbt/net/fifo.h"
#include "mbt/utils/logger.h"

int mbt_msg_receive_handler_piece(struct mbt_net_server *server,
                                  struct mbt_net_client *client)
{
    struct mbt_net_stream *stream = NULL;
    for (size_t i = 0; i < server->streams->size; i++)
    {
        stream = fifo_pop(server->streams);
        fifo_push(server->streams, stream);

        if (stream->client != client)
        {
            stream = NULL;
        }
        else
        {
            break;
        }
    }

    void *v_buf = client->buffer;
    struct mbt_msg *msg = v_buf;

    uint32_t index;
    memcpy(&index, msg->payload, 4);
    index = htonl(index);

    uint32_t begin;
    memcpy(&begin, msg->payload + 4, 4);
    begin = htonl(begin);

    char *data = msg->payload + 8;
    size_t data_len = mbt_msg_length(msg) - 9;

    struct mbt_str *mbt_data = xcalloc(1, sizeof(struct mbt_str));
    mbt_data->size = data_len;
    mbt_data->capacity = data_len;
    mbt_data->data = xcalloc(data_len + 1, sizeof(char));
    memcpy(mbt_data->data, data, data_len);

    logger_buffer("[index (4 bytes) Begin (4 bytes)]", msg->payload, 8);
    logger("Index = %u, Begin = %u\n", index, htonl(begin));

    struct mbt_file_handler *fh = server->ctx->fh;
    mbt_piece_write_block(fh, mbt_data, index, begin);
    mbt_str_free(mbt_data);

    return mbt_net_stream_completed(server, client);
}
