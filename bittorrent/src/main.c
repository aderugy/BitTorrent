#include "main.h"

#include <err.h>
#include <getopt.h>
#include <mbt/utils/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bits/getopt_ext.h"

static struct option l_opts[] = { { "mktorrent", no_argument, 0, 'm' },
                                  { "bind-ip", required_argument, 0, 'b' },
                                  { "bind-port", required_argument, 0, 'p' },
                                  { "pretty-print-torrent-file", no_argument, 0,
                                    'P' },
                                  { "verbose", no_argument, 0, 'v' },
                                  { 0, 0, 0, 0 } };

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        errx(EXIT_FAILURE, "file not specified");
    }

    struct main_options options = {
        .path = argv[argc - 1],
        .ip = NULL,
        .port = NULL,
        .flags = 0,
    };

    int c;
    int opt_idx = 0;
    while ((c = getopt_long(argc - 1, argv, "mb:p:Pv", l_opts, &opt_idx)) != -1)
    {
        switch (c)
        {
        case 'm':
            options.flags |= OPTION_MAKE_TORRENT;
            break;
        case 'b':
            options.ip = optarg;
            break;
        case 'p':
            options.port = optarg;
            break;
        case 'v':
            logger("Test %s\n", "hello");
            logger(NULL, NULL);
            logger("Test %s\n", "hello");
            break;
        case 'P':
            options.flags |= OPTION_PRETTY_PRINT;
            break;
        case '?':
            exit(1);
        default:
            errx(EXIT_FAILURE, "main: unkown option %c", c);
        }
    }

    return options.flags & OPTION_MAKE_TORRENT
        ? main_mktorrent(options)
        : (options.flags & OPTION_PRETTY_PRINT ? main_prettyprint(options)
                                               : main_download(options));
}
