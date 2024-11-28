#include <mbt/net/fifo.h>
#include <stdlib.h>

struct fifo *fifo_init(void)
{
    struct fifo *f = calloc(1, sizeof(struct fifo));

    if (!f)
    {
        return NULL;
    }

    return f;
}

void fifo_clear(struct fifo *fifo)
{
    while (fifo->head)
    {
        fifo_pop(fifo);
    }
}

void fifo_destroy(struct fifo *fifo)
{
    fifo_clear(fifo);
    free(fifo);
}
