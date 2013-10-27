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
#include <tap.h>

static void try(int line, char const *req, MEMREF s, MEMREF a, MEMREF h, MEMREF port, MEMREF p, MEMREF q, MEMREF f);

int
main(void)
{
    plan_tests(13/*tests*/ * 7/*parts*/);

    MEMREF x = NILREF;
    MEMREF s = strref("http");
    MEMREF a = strref("user:pass");
    MEMREF h = strref("sophos.com");
    MEMREF n = strref("443");
    MEMREF p = strref("home/depot");
    MEMREF q = strref("NaN:=0/0");
    MEMREF f = strref("top");

#   undef  TRY
#   define TRY(a1,a2,a3,a4,a5,a6,a7,a8) try(__LINE__,a1,a2,a3,a4,a5,a6,a7,a8)
    TRY("http://sophos.com:443/home/depot?NaN:=0/0#top"         ,s,x,h,n,p,q,f);
    TRY("//user:pass@sophos.com:443/home/depot?NaN:=0/0#top"    ,x,a,h,n,p,q,f);
    TRY("//sophos.com:443/home/depot?NaN:=0/0#top"              ,x,x,h,n,p,q,f);
    TRY(":443/home/depot?NaN:=0/0#top"                          ,x,x,x,n,p,q,f);
    TRY("/home/depot?NaN:=0/0#top"                              ,x,x,x,x,p,q,f);
    TRY("?NaN:=0/0#top"                                         ,x,x,x,x,x,q,f);
    TRY("/home/depot?NaN:=0/0"                                  ,x,x,x,x,p,q,x);
    TRY("/home/depot#top"                                       ,x,x,x,x,p,x,f);
    TRY("/home/depot"                                           ,x,x,x,x,p,x,x);
    TRY("sophos.com"                                            ,x,x,h,x,x,x,x);
    TRY("sophos.com:443"                                        ,x,x,h,n,x,x,x);
    TRY("sophos.com/home/depot"                                 ,x,x,h,x,p,x,x);
    TRY("sophos.com:443/home/depot"                             ,x,x,h,n,p,x,x);

    return exit_status();
}

static void
try(int line, char const *req, MEMREF s, MEMREF a, MEMREF h, MEMREF port, MEMREF p, MEMREF q, MEMREF f)
{
#   undef  CHECK
#   define CHECK(f,n) ok(!refcmp(f, uri.n), #n" %.*s: '%.*s'", (int)f.len, f.ptr, (int)uri.n.len, uri.n.ptr);
    diag("test #%d %s", line, req);
    URIREF uri;
    uri_parts(req, &uri);
    CHECK(s,    scheme);
    CHECK(a,    auth);
    CHECK(h,    host);
    CHECK(port, port);
    CHECK(p,    path);
    CHECK(q,    query);
    CHECK(f,    fragment);
}
