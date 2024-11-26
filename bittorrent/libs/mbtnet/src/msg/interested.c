#include <bits/stdint-uintn.h>
#include <err.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <stdbool.h>
#include <string.h>

int mbt_msg_receive_handler_not_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client)
{
    warnx("Received INTERESTED message !\n");
    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_receive_handler_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client)
{
    warnx("Received INTERESTED message !\n");
    return MBT_HANDLER_SUCCESS;
}

int mbt_msg_send_handler_not_interested(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    unsigned char payload[5];
    memset(payload, 0, 5);
    payload[3] = 1;
    payload[4] = MBT_MAGIC_NOT_INTERESTED;

    MBT_HANDLER_STATUS(sendall(client->fd, payload, 5));
}

int mbt_msg_send_handler_interested(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    printf("Sending interested message\n");
    unsigned char payload[5];
    memset(payload, 0, 5);
    payload[3] = 1;
    payload[4] = MBT_MAGIC_INTERESTED;

    for (size_t i = 0; i < 5; i++)
    {
        printf("%02X ", payload[i]);
    }
    printf("\n");

    client->state = MBT_CLIENT_REQUESTING;
    MBT_HANDLER_STATUS(sendall(client->fd, payload, 5));
}
