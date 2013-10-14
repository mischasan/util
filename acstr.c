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
