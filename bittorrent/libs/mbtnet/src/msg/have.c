#include <bits/stdint-uintn.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <string.h>

int mbt_msg_send_handler_have(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    // Message format:
    // - Length ---- Magic Byte - Piece Index
    // 00 00 00 05 |     04     | XX XX XX XX
    size_t buf_len = sizeof(struct mbt_msg) + 4;
    struct mbt_msg *have_msg = xmalloc(buf_len);

    uint32_t msg_len = 5; // 1 byte type + 4 byte integer payload

    memcpy(have_msg->len, &msg_len, 4);
    have_msg->type = MBT_MAGIC_HAVE;
    memcpy(have_msg->payload, &client->request.index, 4);

    int status = sendall(client->fd, have_msg, buf_len);
    free(have_msg);

    MBT_HANDLER_STATUS(status);
}

int mbt_msg_receive_handler_have(

    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client)

{
    return MBT_HANDLER_SUCCESS;
}
