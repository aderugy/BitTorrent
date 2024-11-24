#include <mbt/utils/hash.h>
#include <mbt/utils/xalloc.h>
#include <openssl/sha.h>

char *sha1(const char *in, size_t len)
{
    const void *vin = in;

    unsigned char *out = xcalloc(SHA_DIGEST_LENGTH + 1, sizeof(char));
    SHA1(vin, len, out);

    void *vout = out;
    return vout;
}
