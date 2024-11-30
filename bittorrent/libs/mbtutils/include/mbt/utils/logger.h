#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>

void logger(const char *s, ...);
void logger_buffer(const char *name, void *buf, size_t len);

#endif // !LOGGER_H
