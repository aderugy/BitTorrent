#include <mbt/utils/logger.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void logger(const char *s, ...)
{
    static bool log = false;

    if (s == NULL)
    {
        log = true;
    }
    else if (log)
    {
        va_list ap;
        va_start(ap, s);
        vprintf(s, ap);
        va_end(ap);
    }
}

void logger_buffer(const char *name, void *buf, size_t len)
{
    unsigned char *c_buf = buf;

    printf("%s (%zu bytes): ", name, len);
    for (size_t i = 0; i < len; i++)
    {
        printf("%02X.", c_buf[i]);
    }
    printf("\n");
}
