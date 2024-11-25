#include <mbt/utils/random.h>
#include <mbt/utils/xalloc.h>
#include <stdlib.h>
#include <time.h>

char *rand_str(size_t length)
{
    long now = time(0);
    srand(now);

    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    char *dest = xcalloc(length + 1, sizeof(char));

    for (size_t i = 0; i < length; i++)
    {
        double rd = rand();
        size_t index = rd / RAND_MAX * (sizeof charset - 1);
        dest[i] = charset[index];
    }

    return dest;
}
