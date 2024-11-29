#include <mbt/net/net_utils.h>
#include <mbt/utils/logger.h>
#include <sys/socket.h>

int sendall(int fd, void *buf, size_t len)
{
    int sent = 0;
    int total_bytes = len;
    char *c_buf = buf;

    logger("Sending %zu bytes to fd %d: ", len, fd);
    for (size_t i = 0; i < len; i++)
    {
        unsigned char c = c_buf[i];
        logger("%02X ", c);
    }
    logger("\n");

    int s = 0;
    while ((s = send(fd, c_buf, total_bytes - sent, 0)) > 0)
    {
        c_buf += s;
        sent += s;
    }

    return s;
}
