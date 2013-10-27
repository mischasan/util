// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

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
