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

// General comment on using string-format IP addrs:
// inet_pton() takes < 0.10 microsec (!) to translate 
// string to inaddr. (< .04 usec for IPv4).

#include <errno.h>
#include <netinet/in.h>     // INADDR_LOOPBACK
#include <tap.h>
#include "msutil.h"

int main(void)
{
    plan_tests(11);

    int     port = 0xAA00 | (rand() & 255);
    int     sx = udp_open("", 0);
    ok(sx > 0, "created dynamic-port udp socket %s", errname[errno]); 
    int     sy = udp_open("", port);
    ok(sy > 0, "created static-port udp socket %s", errname[errno]); 

    IPSTR   rip;
    int     rport;
    char    rcvbuf[99];
    char    sndbuf[] = "hello, world";
    int     ret;

    ret = sock_addr(sy, rip, &rport, rcvbuf, sizeof rcvbuf);
    ok(ret == 0 && !strcmp(rip, "0.0.0.0") && rport == port && !strcmp(rcvbuf, rip),
        "sock_addr(udp): %s:%u", rip, rport);

    ret = udp_send(sx, sndbuf, sizeof(sndbuf), "127.0.0.1", port);
    ok(ret == sizeof(sndbuf), "udp_send(127.0.0.1): %d %s", ret, errname[errno]);

    ret = udp_recv(sy, rcvbuf, sizeof(rcvbuf), rip, &rport);
    
    ok(ret == sizeof(sndbuf), "udp_recv: %d %s", ret, errname[errno]);
    ok(!strcmp(rip, "127.0.0.1"), "received from %s port=%d", rip, rport);

    close(sx);
    close(sy);

    sx = udp_open("::1", 0);
    skip_if(sx < 0 && errno == EAFNOSUPPORT, 5, "tests that require IPv6 support") {
        ok(sx > 0, "created dynamic-port IPv6 udp socket %s", errname[errno]); 
        sy = udp_open("::1", port);
        ok(sy > 0, "created static-port udp socket %s", errname[errno]); 

        ret = udp_send(sx, sndbuf, sizeof(sndbuf), "::1", port);
        ok(ret == sizeof(sndbuf), "udp_send(::1): %d %s", ret, errname[errno]);

        ret = udp_recv(sy, rcvbuf, sizeof(rcvbuf), rip, &rport);
        
        ok(ret == sizeof(sndbuf), "udp_recv(IPv6): %d %s", ret, errname[errno]);
        ok(!strcmp(rip, "::1"), "received from %s port=%d", rip, rport);
    }

    return exit_status();
}
