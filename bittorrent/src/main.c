#include "main.h"

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "bits/getopt_ext.h"

#define OPTION_VERBOSE 1
#define OPTION_PRETTY_PRINT 2

static struct option l_opts[] = { { "mktorrent", required_argument, 0, 'm' },
                                  { "bind-ip", required_argument, 0, 'b' },
                                  { "bind-port", required_argument, 0, 'p' },
                                  { "pretty-print-torrent-file", no_argument, 0,
                                    'P' },
                                  { "verbose", no_argument, 0, 'v' },
                                  { 0, 0, 0, 0 } };

int main(int argc, char *argv[])
{
    struct main_options options = { 0, NULL, NULL, NULL };

    int c;
    int opt_idx = 0;
    while ((c = getopt_long(argc, argv, "m:b:p:Pv", l_opts, &opt_idx)) != -1)
    {
        switch (c)
        {
        case 'm':
            if (options.ip || options.port)
            {
                errx(EXIT_FAILURE, "options: cant make and leech torrent");
            }
            options.path = optarg;
            break;
        case 'b':
            if (options.path)
            {
                errx(EXIT_FAILURE, "options: cant make and leech torrent");
            }
            options.ip = optarg;
            break;
        case 'p':
            if (options.path)
            {
                errx(EXIT_FAILURE, "options: cant make and leech torrent");
            }
            options.port = optarg;
            break;
        case 'v':
            options.flags |= OPTION_VERBOSE;
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

    return options.path
        ? (options.flags & OPTION_PRETTY_PRINT ? main_prettyprint(options)
                                               : main_mktorrent(options))
        : main_download(options);
}
