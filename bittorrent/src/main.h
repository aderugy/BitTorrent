#ifndef MAIN_H
#define MAIN_H

#define OPTION_VERBOSE 1
#define OPTION_PRETTY_PRINT 2
#define OPTION_MAKE_TORRENT 4

struct main_options
{
    int flags;
    char *path;
    char *ip;
    char *port;
};

int main_mktorrent(struct main_options options);
int main_download(struct main_options options);
int main_prettyprint(struct main_options options);

#endif // !MAIN_H
