#include <bits/stdint-uintn.h>
#include <mbt/be/bencode.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <string.h>

static void putain_de_sa_race(uint32_t n, unsigned char *buf)
{
    void *v_ptr = &n;
    unsigned char *c_ptr = v_ptr;

    for (size_t i = 0; i < 4; i++)
    {
        buf[3 - i] = c_ptr[i];
    }
}

int mbt_msg_receive_handler_request(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    void *v_buf = client->buffer;
    struct mbt_msg *msg = v_buf;

    MBT_EXTRACT_UINT32_T(msg, 5, index)
    MBT_EXTRACT_UINT32_T(msg, 9, begin)
    MBT_EXTRACT_UINT32_T(msg, 13, length)

    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_send_handler_request(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_stream *stream)
{
    /*
     * 17 bytes message
     *  LENGTH (4)        TYPE (1)     Index (4)      Begin (4)   Length (4)
     * 00 00 00 0D MBT_MAGIC_REQUEST XX XX XX XX | XX XX XX XX | XX XX XX XX
     */
    unsigned char buf[17];
    memset(buf, 0, 17);
    buf[3] = 13;
    buf[4] = MBT_MAGIC_REQUEST;

    putain_de_sa_race(stream->index, buf + 5);
    putain_de_sa_race(stream->begin, buf + 9);
    putain_de_sa_race(stream->length, buf + 13);
    stream->status = STREAM_STARTED;

    int status = sendall(stream->client->fd, buf, 17);
    if (status == MBT_HANDLER_SUCCESS)
    {
        stream->client->state = MBT_CLIENT_DOWNLOADING;
    }

    MBT_HANDLER_STATUS(status);
}
