#include <err.h>
#include <mbt/net/net.h>
#include <stdlib.h>

int mbt_msg_receive_handler_cancel(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client)
{
    errx(EXIT_FAILURE, "mbt_msg_receive_handler_cancel: not implemented");
}

int mbt_msg_send_handler_cancel(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client)
{
    errx(EXIT_FAILURE, "mbt_msg_receive_handler_cancel: not implemented");
}
