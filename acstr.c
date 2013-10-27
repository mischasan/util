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

// acstr: format byte-buffer as a C string.
#include <ctype.h>
#include "msutil.h"

static char esc[256] = {
       0 , 0 , 0 , 0 , 0 , 0 , 0 ,'a','b','t','n','v','f','r', 0 , 0 ,
       0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
       0 , 0 ,'"', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
       0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
       0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
       0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,'\\'
};

char *acstr(char const *buf, int len)
{
    int        i, wid = 3;
    //  Trailing NULs are ignored.
    while (len > 0 && !buf[len-1]) --len;

    for (i = 0; i < len; ++i)
        wid += esc[(unsigned char)buf[i]] ? 2 : isgraph(buf[i]) ? 1 : 4;
    char        *ret = malloc(wid + 1), *cp = ret, ec;
    *cp++ = '"';
    for (i = 0; i < len; i++) {
        if ((ec = esc[(unsigned char)buf[i]]))
            *cp++ = '\\', *cp++ = ec;
        else if (buf[i] == ' ' || isgraph(buf[i]))
            *cp++ = buf[i];
        else
            cp += sprintf(cp, "\\x%2.2X", (unsigned char)buf[i]);
    }

    *cp++ = '"';
    *cp = 0;
    return      ret;
}
