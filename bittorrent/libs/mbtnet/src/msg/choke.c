#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/net/net_utils.h>
#include <stdbool.h>
#include <string.h>

#include "mbt/net/msg.h"

int mbt_msg_receive_handler_unchoked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    client->choked = false;
    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_receive_handler_choked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    client->choked = true;
    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_send_handler_unchoked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    printf("Sending unchoked message\n");
    unsigned char payload[5];
    memset(payload, 0, 5);
    payload[3] = 1;
    payload[4] = MBT_MAGIC_UNCHOKE;

    for (size_t i = 0; i < 5; i++)
    {
        printf("%02X ", payload[i]);
    }
    printf("\n");

    MBT_HANDLER_STATUS(sendall(client->fd, payload, 5));
}

int mbt_msg_send_handler_choked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    client->choked = true;
    return MBT_HANDLER_SUCCESS;
}
