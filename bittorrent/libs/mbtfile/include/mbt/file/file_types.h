#ifndef FILE_TYPES_H
#define FILE_TYPES_H

#include <mbt/be/types_mbtbe.h>
#include <mbt/file/file_handler.h>

struct mbt_file_handler
{
    struct mbt_torrent *torrent;
};

#endif // !FILE_TYPES_H
