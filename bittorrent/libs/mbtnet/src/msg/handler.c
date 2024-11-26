#include <mbt/net/msg_handler.h>

#include "mbt/net/msg.h"

#define MBT_NB_RECEIVE_HANDLERS 9

static int (*MBT_RECEIVE_HANDLERS[])(struct mbt_net_server *,
                                     struct mbt_net_client *) = {
    mbt_msg_receive_handler_choked,     mbt_msg_receive_handler_unchoked,
    mbt_msg_receive_handler_interested, mbt_msg_receive_handler_not_interested,
    mbt_msg_receive_handler_have,       mbt_msg_receive_handler_bitfield,
    mbt_msg_receive_handler_request,    mbt_msg_receive_handler_piece,
    mbt_msg_receive_handler_cancel
};

int mbt_msg_handler(struct mbt_net_server *server,
                    struct mbt_net_client *client, struct mbt_msg *msg)
{
    if (mbt_msg_length(msg) == 0) // Keepalive message
    {
        return mbt_msg_handler_keepalive(server, client);
    }

    if (msg->type >= MBT_NB_RECEIVE_HANDLERS) // Index out of bounds
    {
        return MBT_HANDLER_CLIENT_ERROR;
    }

    // Associated handler
    return MBT_RECEIVE_HANDLERS[msg->type](server, client);
}
