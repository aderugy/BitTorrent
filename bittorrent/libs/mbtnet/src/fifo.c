#include <mbt/net/fifo.h>
#include <stdio.h>
#include <stdlib.h>

size_t fifo_size(struct fifo *fifo)
{
    struct list *h = fifo->head;
    size_t c = 0;

    while (h)
    {
        c++;
        h = h->next;
    }

    return c;
}

void fifo_push(struct fifo *fifo, void *elt)
{
    struct list *l = calloc(1, sizeof(struct list));

    if (!l)
    {
        return;
    }

    l->data = elt;

    if (!fifo->head || !fifo->tail)
    {
        fifo->head = l;
        fifo->tail = l;
    }
    else
    {
        fifo->tail->next = l;
        fifo->tail = l;
    }

    fifo->size++;
}

void *fifo_head(struct fifo *fifo)
{
    return fifo->head->data;
}

void *fifo_pop(struct fifo *fifo)
{
    if (!fifo || !fifo->head)
    {
        return NULL;
    }

    struct list *h = fifo->head;
    if (h == fifo->tail)
    {
        fifo->tail = NULL;
    }

    fifo->head = h->next;
    void *data = h->data;
    free(h);

    fifo->size--;
    return data;
}

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
