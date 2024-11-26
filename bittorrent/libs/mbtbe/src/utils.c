#include <err.h>
#include <mbt/be/bencode.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mbt/be/types_mbtbe.h"

struct mbt_be_node *mbt_be_dict_get(struct mbt_be_node *dict, const char *key)
{
    struct mbt_be_pair **keys = dict->v.dict;

    for (size_t i = 0; keys[i]; i++)
    {
        if (strcmp(key, keys[i]->key.data) == 0)
        {
            return keys[i]->val;
        }
    }

    return NULL;
}
