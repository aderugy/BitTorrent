#include <bits/stdint-uintn.h>
#include <mbt/net/msg.h>
#include <mbt/net/msg_handler.h>
#include <mbt/net/net.h>
#include <mbt/net/net_utils.h>
#include <mbt/utils/xalloc.h>
#include <stdbool.h>
#include <string.h>

int mbt_msg_handler_keepalive(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client)
{
    unsigned char buffer[4];
    memset(buffer, 0, 4);

    MBT_HANDLER_STATUS(sendall(client->fd, buffer, 4));
}
