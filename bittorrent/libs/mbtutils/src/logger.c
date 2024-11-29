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
