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

    struct addrinfo *gaip = NULL, *aip,
           hint = { AI_NUMERICSERV | AI_CANONNAME, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
                    /*addrlen*/0, /*addr*/0, /*canonname*/0, /*next*/0 };

    while (*++argv) {
        gaip = 0;
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
