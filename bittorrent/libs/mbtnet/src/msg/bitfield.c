#include <bits/stdint-uintn.h>
#include <err.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int mbt_msg_receive_handler_bitfield(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    if (client->state != MBT_CLIENT_HANDSHAKEN)
    {
        warnx("Receiving bitfield before handshake / more than once");
        return MBT_HANDLER_REQUEST_CLOSE;
    }

    printf("recv: bitfield: %d\n", client->fd);

    void *v_msg = client->buffer;
    struct mbt_msg *msg = v_msg;

    uint32_t bytes = mbt_msg_length(msg) - 1;
    client->bitfield = xcalloc(bytes + 1, sizeof(bool));

    for (uint32_t i = 0; i < bytes; i++)
    {
        client->bitfield[i] = (msg->payload[i] & (1 << (7 - (i % 8)))) > 0;

        printf("%c", client->bitfield[i] ? '1' : '0');
        printf(" ");
    }
    printf("\n\n");

    client->state = MBT_CLIENT_BITFIELD_RECEIVED;
    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_send_handler_bitfield(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    unsigned char buf[6];
    memset(buf, 0, 6);
    buf[3] = 2;
    buf[4] = MBT_MAGIC_BITFIELD;
    buf[5] = 0;

    MBT_HANDLER_STATUS(sendall(client->fd, buf, 6))
}
