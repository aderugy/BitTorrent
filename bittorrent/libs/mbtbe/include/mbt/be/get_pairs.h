#ifndef GET_PAIRS_H
#define GET_PAIRS_H

#include <mbt/be/torrent.h>
#include <stdbool.h>

bool is_dir(const char *path);

struct mbt_be_pair *transform_to_pair(struct mbt_be_node *node, char *value);

uint64_t get_file_size(const char *filename);

struct mbt_be_pair *get_pieces_length();

struct mbt_be_pair *get_name(const char *path);

struct mbt_be_pair *get_length(const char *path);

struct mbt_be_pair *get_creation_date(const char *path);

struct mbt_be_pair *get_announce(void);
#endif // GET_PAIRS_H
