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

#include "msutil.h"

// RFC2396 #4.3 says 
//  [unres]+:// means scheme:...
//  [unres]+[:/] means host[:port][/path]
//  ^// means //[user[:pass]@]host[:port]...
//  ^/[^/] means /path...
// 
// hostname:$  (empty port) is legal.
// There are schemes containing [-.0-9]

static int tok[256] = {
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // '\0'
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // '#', '/'
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, // ':', '?'
};

// ":/?#\0" -> 1,2,3,4,5
// Note that there are other reserved chars: $%&+=.@[]
//  But other than the special basic-auth case, they don't matter here.
static inline char const *find(char const *tgt, int min)
{
    while (tok[*tgt & 0xFF] < min) ++tgt;
    return  tgt;
}

void
uri_parts(char const *request, URIREF *uri)
{
    char const  *ap, *cp = find(request, 1);   // brk ":/?#\0"
    memset(uri, 0, sizeof(URIREF));

    if (cp > request && cp[0] == ':' && cp[1] == '/' && cp[2] == '/') {
        uri->scheme = (MEMREF){request, cp - request};
        cp += 3;
    } else {
        cp = request;
    }

    if (cp[0] == '/' && cp[1] == '/') {
        // Do not use find(): auth may contain ':'
        // and password may contain anything except '@'
        //  and query or fragment may contain "@"??
        if ((ap = strchr(cp += 2, '@'))) { // BasicAuth
            uri->auth = (MEMREF){cp, ap - cp};
            cp = ap + 1;
        }
    }

    uri->host.ptr = cp;
    cp = find(cp, 1);   // brk ":/?#\0"
    if (!(uri->host.len = cp - uri->host.ptr))
        uri->host.ptr = NULL;

    if (*cp == ':') {
        uri->port.ptr = ++cp;
        cp = find(cp, 2);    // brk "/?#\0"
        uri->port.len = cp - uri->port.ptr;
    }

    if (*cp == '/') {
        uri->path.ptr = ++cp;
        cp = find(cp, 3);    // brk "#?\0"
        uri->path.len = cp - uri->path.ptr;
    }

    if (*cp == '?') {
        uri->query.ptr = ++cp;
        cp = find(cp, 4);    // brk "#\0"
        uri->query.len = cp - uri->query.ptr;
    }

    if (*cp == '#') {
        uri->fragment.len = strlen(uri->fragment.ptr = cp + 1);
    }
}
