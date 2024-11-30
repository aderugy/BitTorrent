#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include <errno.h>
#include <mbt/net/msg.h>
#include <mbt/net/net.h>

#define MBT_HANDLER_CLIENT_ERROR -1
#define MBT_HANDLER_SUCCESS 0
#define MBT_HANDLER_REQUEST_CLOSE 1
#define MBT_HANDLER_IGNORE 2

#define MBT_EXTRACT_UINT32_T(msg, offset, varname)                             \
    uint32_t varname;                                                          \
    memcpy(&varname, msg->payload + offset, 4);

#define MBT_HANDLER_STATUS(status)                                             \
    if (status < 0)                                                            \
    {                                                                          \
        if (errno != EINPROGRESS)                                              \
        {                                                                      \
            return MBT_HANDLER_IGNORE;                                         \
        }                                                                      \
                                                                               \
        return MBT_HANDLER_CLIENT_ERROR;                                       \
    }                                                                          \
                                                                               \
    return MBT_HANDLER_SUCCESS;

/*
 * Defines all the methods to handle properly messages.
 * Receive handlers ignore unimplemented messages.
 *
 * Return a MBT_HANDLER family status code.
 */

int mbt_msg_handler(struct mbt_net_server *server,
                    struct mbt_net_client *client, struct mbt_msg *msg);

// BITFIELD HANDLERS
int mbt_msg_receive_handler_bitfield(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_send_handler_bitfield(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

// CHOKE HANDLERS
int mbt_msg_receive_handler_unchoked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_receive_handler_choked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_send_handler_unchoked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_send_handler_choked(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

// HAVE HANDLERS
int mbt_msg_send_handler_have(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_stream *stream);

int mbt_msg_receive_handler_have(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client);

// INTERESTED HANDLERS
int mbt_msg_send_handler_not_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client);

int mbt_msg_send_handler_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client);

int mbt_msg_receive_handler_not_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client);

int mbt_msg_receive_handler_interested(
    __attribute((unused)) struct mbt_net_server *server,
    __attribute((unused)) struct mbt_net_client *client);

// KEEPALIVE HANDLER (works both ways)
int mbt_msg_handler_keepalive(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

// PIECE HANDLER
int mbt_msg_receive_handler_piece(struct mbt_net_server *server,
                                  struct mbt_net_client *client);

// REQUEST HANDLER
int mbt_msg_receive_handler_request(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_send_handler_request(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_stream *stream);

// CANCEL HANDLER
int mbt_msg_receive_handler_cancel(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

int mbt_msg_send_handler_cancel(
    __attribute((unused)) struct mbt_net_server *server,
    struct mbt_net_client *client);

#endif // !MSG_HANDLER_H
