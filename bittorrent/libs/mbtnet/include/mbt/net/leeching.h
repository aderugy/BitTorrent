#ifndef MBT_NET_LEECHING_H
#define MBT_NET_LEECHING_H

////////////////////////////////////////////////////////////////////////////////
/// THIS FILE WILL BE OVERWRITTEN                                            ///
////////////////////////////////////////////////////////////////////////////////

// mbtnet
#include <mbt/net/context.h>
#include <mbt/net/tracker.h>

/*
 * ALGORITHM
 * 1    Connect to peer
 *      1.1     Connection succeeds or is waiting: create client
 *      1.2     Connection fails: go next
 * 2    Processing events
 *
 *      2.1     MBT_CLIENT_WAITING_CONNECTION
 *              2.1.1   Waiting for client to accept connection. Nothing to do
 *
 *      2.2     MBT_CLIENT_CONNECTED
 *              2.2.1   The client accepted the connection. Send the handshake
 *
 *      2.3     MBT_CLIENT_WAITING_HANDSHAKE
 *              2.3.1   The handshake is sent. Waiting for their response
 *
 *      2.4     MBT_CLIENT_HANDSHAKEN
 *              2.4.1   The client sent the handshake and accepted ours.
 *              2.4.2   We are now waiting for the bitfield.
 *
 *      2.5     MBT_CLIENT_BITFIELD_RECEIVED
 *              2.5.1   Send the interested message
 *              2.5.2   Register the client to the block allocator
 *              2.5.3   On unchoke message, mark the client as MBT_CLIENT_READY
 *
 * EVENT PROCESSING
 */
void mbt_leech(struct mbt_net_context *ctx);

#endif // !MBT_NET_LEECHING_H
