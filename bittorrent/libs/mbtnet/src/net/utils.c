#include <mbt/net/net_utils.h>
#include <sys/socket.h>

int sendall(int fd, void *buf, size_t len)
{
    int sent = 0;
    int total_bytes = len;

    char *c_buf = buf;

    int s;
    while ((s = send(fd, c_buf, total_bytes - sent, 0)) > 0)
    {
        c_buf += s;
        sent += s;
    }

    return s;
}
