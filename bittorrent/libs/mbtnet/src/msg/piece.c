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

/*
 * Algorithm:
 * 1/ Parse the message
 * 2/ Check that the block matches the file handler specs
 * 3/ Write the block in memory
 * 4/ Check if the piece is complete and valid
 *      a/ The piece is valid:
 *          1/ Mark piece as success
 *          2/ Write it to the disk
 *          3/ Send 'have' message
 *          4/ Find another piece for the peer
 *      b/ The piece is invalid: close the client and mark the piece as
 *          available
 *      c/ The piece is downloading: return normally
 */
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

    uint32_t begin;
    memcpy(&begin, msg->payload + 4, 4);

    char *data = msg->payload + 8;
    size_t data_len = mbt_msg_length(msg) - 8;
    if (begin % MBT_BLOCK_SIZE)
    {
        warnx("mbt_net_msg_receive_handler_piece: begin is not a multiple of "
              "MBT_BLOCK_SIZE");
        return MBT_HANDLER_REQUEST_CLOSE;
    }
    if (data_len > MBT_BLOCK_SIZE)
    {
        warnx("mbt_net_msg_receive_handler_piece: data_len > MBT_BLOCK_SIZE");
        return MBT_HANDLER_REQUEST_CLOSE;
    }

    struct mbt_str *mbt_data = xcalloc(1, sizeof(struct mbt_str));
    mbt_data->size = data_len - 1;
    mbt_data->capacity = data_len - 1;
    mbt_data->data = xcalloc(data_len, sizeof(char));
    memcpy(mbt_data->data, data, data_len - 1);

    struct mbt_file_handler *fh = server->ctx->fh;
    mbt_piece_write_block(fh, mbt_data, index, begin);
    mbt_str_free(mbt_data);

    int piece_status = mbt_piece_check(fh, stream->index);
    if (piece_status == MBT_PIECE_VALID)
    {
        if (!mbt_piece_write(fh, stream->index))
        {
            errx(EXIT_FAILURE,
                 "mbt_net_msg_receive_handler_piece: Failed to write piece");
        }

        printf("C'EST TROP CARRE FREROT\n");
        // TODO: Find another piece to request
        client->state = MBT_CLIENT_COMPLETED;
        return MBT_HANDLER_SUCCESS;
    }
    else if (piece_status == MBT_PIECE_INVALID)
    {
        // TODO: Reset piece and find another peer
        return MBT_HANDLER_CLIENT_ERROR;
    }

    return MBT_HANDLER_SUCCESS;
}
