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
#include <netdb.h>          // getaddrinfo
#include <arpa/inet.h>      // inet_ntop
#include <sys/socket.h>     // AF_INET etc
#ifdef __FreeBSD__
#   include <netinet/in.h>  // IPPROTO_TCP
#endif

int main(int argc, char **argv)
{
    if (argc == 1) usage("host... -- display getaddrinfo results");

    struct addrinfo crap, *gaip = NULL, *aip,
           hint = { AI_NUMERICSERV | AI_CANONNAME, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
                    /*addrlen*/0, /*addr*/0, /*canonname*/0, /*next*/0 };

    while (*++argv) {
        gaip = &crap;
        int ret = getaddrinfo(*argv, 0, &hint, &gaip);
        if (ret)  {
            fprintf(stderr, "%s error:%d %s\n", *argv, ret, eainame[-ret]);
        } else if (gaip) {

            printf("%s %s\n", *argv, 
                    gaip->ai_canonname && strcmp(gaip->ai_canonname, *argv)
                        ? gaip->ai_canonname : "");

            for (aip = gaip; aip; aip = aip->ai_next) {
                IPSTR    ip = {};
                inet_ntop(aip->ai_family, 
                            aip->ai_family == AF_INET6 
                                ? (void*)&((struct sockaddr_in6*)aip->ai_addr)->sin6_addr
                                : (void*)&((struct sockaddr_in *)aip->ai_addr)->sin_addr, 
                            ip, sizeof(ip));
                printf("\t%s\n", ip);
            }

            freeaddrinfo(gaip);
        }
    }

    return 0;
}
