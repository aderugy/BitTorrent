#ifndef FIFO_H
#define FIFO_H

#include <stddef.h>

struct list
{
    void *data;
    struct list *next;
};

struct fifo
{
    struct list *head;
    struct list *tail;
    size_t size;
};

struct fifo *fifo_init(void);
size_t fifo_size(struct fifo *fifo);
void fifo_push(struct fifo *fifo, void *elt);
void *fifo_head(struct fifo *fifo);
void *fifo_pop(struct fifo *fifo);
void fifo_clear(struct fifo *fifo);
void fifo_destroy(struct fifo *fifo);

#endif /* !FIFO_H */
