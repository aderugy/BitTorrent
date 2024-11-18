#ifndef XALLOC_H
#define XALLOC_H

#include <stddef.h>

/**
** \brief Malloc wrapper that exits on failure.
**
** \param size The size to malloc.
** \return The malloc return.
*/
void *xmalloc(size_t size);

/**
** \brief Calloc wrapper that exits on failure.
**
** \param nmemb The number of elements.
** \param size The size of an element.
** \return The calloc return.
*/
void *xcalloc(size_t nmemb, size_t size);

/**
** \brief Realloc wrapper that exits on failure.
**
** \param ptr The mem pointer.
** \param size The size to realloc.
** \return The realloc return.
*/
void *xrealloc(void *ptr, size_t size);

#endif /* !XALLOC_H */
