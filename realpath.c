// Linux does not have realpath command !?

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int     i;
    char    path[argc][8192];

    if (argc > 1 && !strcmp(argv[1], "-q"))
        --argc, ++argv, fclose(stderr);

    if (argc < 2)
        return fputs("usage: realpath [-q] path [...]\n", stderr), 1;

    for (i = 1; i < argc; ++i)
        if (!realpath(argv[i], path[i]))
            return perror(argv[i]), 1;

    for (i = 1; i < argc; ++i)
        puts(path[i]);

    return  0;
}
