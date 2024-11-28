#ifndef MSG_H
#define MSG_H

#include <mbt/file/file_types.h>
#include <mbt/net/net.h>
#include <mbt/net/net_types.h>
#include <mbt/utils/utils.h>
#include <stdbool.h>

#define MBT_PEER_ID_PREFIX "-MB2021-"

#define MBT_HANDSHAKE_PROTOCOL_LENGTH 19
#define MBT_HANDSHAKE_PROTOCOL "BitTorrent protocol"

#define STREAM_SUCCESS 0
#define STREAM_FULL 1
#define STREAM_NO_BLOCK_AVAILABLE 2

struct mbt_msg_handshake
{
    char len; // MBT_HANDSHAKE_PROTOCOL_LENGTH
    char protocol[MBT_HANDSHAKE_PROTOCOL_LENGTH]; // MBT_HANDSHAKE_PROTOCOL
    char extension[8]; // UNUSED NULL BYTES
    char info_hash[MBT_H_LENGTH];
    char peer_id[PEER_ID_LENGTH];
};

enum mbt_msg_type
{
    MBT_MAGIC_CHOKE = 0,
    MBT_MAGIC_UNCHOKE = 1,
    MBT_MAGIC_INTERESTED = 2,
    MBT_MAGIC_NOT_INTERESTED = 3,
    MBT_MAGIC_HAVE = 4,
    MBT_MAGIC_BITFIELD = 5,
    MBT_MAGIC_REQUEST = 6,
    MBT_MAGIC_PIECE = 7,
    MBT_MAGIC_CANCEL = 8
};

struct mbt_msg
{
    unsigned char len[4];
    unsigned char type;
    char payload[];
};

void mbt_msg_print(struct mbt_msg *msg);
void mbt_msg_handshake_print(struct mbt_msg_handshake *msg);
uint32_t mbt_msg_length(struct mbt_msg *msg);
void mbt_msg_discard(struct mbt_net_client *client, size_t size);

void mbt_net_server_process_event(struct mbt_net_server *server,
                                  struct mbt_net_client **clients);

bool mbt_msg_verify_handshake(struct mbt_net_context *ctx,
                              struct mbt_msg_handshake hs) MBT_NONNULL(1);

void mbt_msg_write_handshake(struct mbt_net_context *ctx,
                             struct mbt_msg_handshake *hs) MBT_NONNULL(1, 2);

bool mbt_msg_process(struct mbt_net_server *server,
                     struct mbt_net_client *client,
                     char buffer[MBT_NET_BUFFER_SIZE], int read)
    MBT_NONNULL(1, 2);

#endif /* ifndef MSG_H */
