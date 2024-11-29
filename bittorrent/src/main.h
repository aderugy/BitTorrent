#ifndef MAIN_H
#define MAIN_H

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
