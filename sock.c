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

// sock: ya wrapper for network calls
// - interface is all simple "C" types: native ints and strings.
// - EINTR-proofed. Since every sock/udp call resets errno,
//      then errno==EINTR if there was a signal, even if the
//      sock call returns a nonnegative.
// - supports NOWAIT I/O, IPv6.
//XXX: fcntl O_ASYNC for lovers of SIGIO :-|
//XXX make the (sendfd) interface into the only interface?
//XXX add writev-type interface.
//XXX Need a way to determine HAS_ACCEPT4

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>          // sprintf
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <netdb.h>          // gethostbyname
#include <netinet/in.h>     // sockaddr_in
#include <netinet/tcp.h>    // TCP_NODELAY
#include <arpa/inet.h>      // inet_ntop
#include <sys/un.h>
#include <sys/wait.h>
#if __BSD_VISIBLE
#   include <sys/socket.h>     // cmsghdr
#endif

#include "sock.h"
#if defined(linux) && defined(_GNU_SOURCE)
#   undef HAS_ACCEPT4       // XXX
#endif

#ifdef _WIN32
#   define FD_CLOEXEC   (-1)
#   define F_GETFL      (-1)
#   define F_SETFD      (-1)
#   define F_SETFL      (-1)
#   define IPPROTO_TCP  (-1)
#   define O_NONBLOCK   (-1)
#   define fcntl(a,b,c) (0)    /*!!*/
// Windows has no unistd.h:
extern int unlink(const char *);

#else   // Linux, FreeBSD, Darwin ...
#   define closesocket(x) close(x)

typedef struct { struct cmsghdr c; int fd; } FDCMSG;

static const struct cmsghdr fdc_hdr = {
    sizeof(FDCMSG), SOL_SOCKET, SCM_RIGHTS
};
#endif

// Internal types
typedef struct addrinfo     ADRINFO;
typedef struct sockaddr_in  IN4ADDR;
typedef struct sockaddr_in6 IN6ADDR;
typedef struct sockaddr     SOADDR;
typedef struct sockaddr_un  UNADDR;
typedef uint8_t             IP6ADDR[16];    //HAHA make this __m128?

typedef union { IN4ADDR in4; IN6ADDR in6; } INxADDR;
// Linux and FreeBSD put fields common to (IPv4,IPv6) at the same offsets:
#define inx_family in4.sin_family
#define inx_port   in4.sin_port

// dyninit: ensure cloexec supported in runtime env.
static void dyninit(void);
static void ininfo(INxADDR const*, IPSTR, int*pport);
static int  ininit(INxADDR*, IPSTR const, int port);
static int  sockit(int domain);
static int  uninit(UNADDR *, char const *path);

static int      sock_cloexec = 1, cmsg_cloexec, has_accept4;

static int eclose(int fd)
{ int e = errno; closesocket(fd); errno = e; return -1; }

static inline int BIT(int u, int m, int op)
{ return op ? u | m : u & ~m; }

int
sock_bind(IPSTR const ip, int port)
{
    dyninit();

    INxADDR addr;
    int addrlen = ininit(&addr, ip, port);
    if (addrlen < 0)
        return -1;

    int fd = sockit(addr.inx_family);
    if (fd < 0)
        return -1;
#   ifdef IPV6_V6ONLY
    if (addr.inx_family == AF_INET6) {
        int     no = 0;
        if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)))
            return eclose(fd);
    }
#   endif
    //XXX support bindresvport(), i.e. ask for an arb port in 512..1023?
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &fd, sizeof fd)
        || bind(fd, (SOADDR*)&addr, addrlen)
        || listen(fd, 5)
        ? eclose(fd) : fd;
}

int
sock_accept(int skt)
{
    dyninit();

    int fd;
    do fd =
#           ifdef HAS_ACCEPT4
            has_accept4 ? accept4(skt, NULL, NULL, sock_cloexec) :
#           endif
            accept(skt, NULL, NULL);
    while (fd < 0 && errno == EINTR);

    return fd >= 0 && !has_accept4 && fcntl(fd, F_SETFD, FD_CLOEXEC)
            ? eclose(fd) : fd;
}

int
sock_create(char const *path)
{
    dyninit();

    UNADDR sun;
    int len = uninit(&sun, path);
    if (!len)
        return errno = EINVAL, -1;

    if (unlink(path) && errno != ENOENT)
        return -1;

    int fd = sockit(AF_UNIX);
    if (fd < 0)
        return -1;

    return (   setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &fd, sizeof fd)
            || bind(fd, (SOADDR*)&sun, sizeof sun.sun_family + len)
            || listen(fd, 5)
           ) ? eclose(fd) : fd;
}

int
sock_open(char const *path)
{
    dyninit();

    struct stat st;
    UNADDR sun;
    int fd, ret, len = uninit(&sun, path);
    if (!len || stat(path, &st) || !S_ISSOCK(st.st_mode))
        return errno = EINVAL, -1;

    if (0 > (fd = sockit(AF_UNIX)))
        return -1;

    do ret = connect(fd, (SOADDR*)&sun, SUN_LEN(&sun));
    while (ret < 0 && errno == EINTR);

    return ret < 0 && errno != EAGAIN ? eclose(fd) : fd;
}

void
sock_close(int skt)
{ closesocket(skt); }

int
sock_connect(const char *host, int port, int nowait)
{
    dyninit();

    char sport[7];
    sprintf(sport, "%hu", (uint16_t)port);
    ADRINFO *aip, hint = {
        AI_NUMERICSERV, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
        /*addrlen */0, /*addr */0, /*canonname */0, /*next */0
    };

    if (getaddrinfo(host, sport, &hint, &aip))
        return -1;

    // "ai_family" usable here because PF_INET=AF_INET, PF_INET6=AF_INET6.
    int fd = sockit(aip->ai_family), ret = -1;
    if (fd >= 0
        && !(nowait && sock_setopt(fd, NOWAIT, 1))
        && !setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &fd, sizeof fd)) {

        do ret = connect(fd, aip->ai_addr, aip->ai_addrlen);
        while (ret < 0 && errno == EINTR);
    }

    freeaddrinfo(aip);
    return ret < 0 && errno != EINPROGRESS ? eclose(fd) : fd;
}

int
sock_recv(int skt, char *buf, int size)
{
    dyninit();

    int ret;
    do ret = recv(skt, buf, size, 0);
    while (ret < 0 && errno == EINTR);

    return ret;
}

int
sock_send(int skt, char const *buf, int size)
{
    dyninit();

    int ret;
    do ret = send(skt, buf, size, 0);
    while (ret < 0 && errno == EINTR);

    return ret;
}

int
sock_recvfd(int skt, int *pfd, char *buf, int size)
{
    dyninit();

    int ret;
    char x;
    FDCMSG ctl = { {sizeof ctl, 0, 0} , -1 };
    struct iovec iov[2] = { {&x, sizeof x} , {buf, size} };
    struct msghdr msg = { 0, 0, iov, 1 + (buf && size), &ctl, sizeof ctl, 0 };

    do ret = recvmsg(skt, &msg, cmsg_cloexec);
    while (ret < 0 && errno == EINTR);

    if (ret >= 0 && !cmsg_cloexec && fcntl(ctl.fd, F_SETFD, FD_CLOEXEC))
        return close(ctl.fd), -1;

    if (pfd)
        *pfd = ctl.fd;

    return ret < 0 ? ret
        : memcmp(&ctl.c, &fdc_hdr, sizeof fdc_hdr) ? errno = ENOMSG, -1
        : ret - (int) sizeof(x) * (ret > 0);
}

int
sock_sendfd(int skt, int fd, char const *buf, int size)
{
    dyninit();

    char          x = 0;
    FDCMSG        ctl = { fdc_hdr, fd };
    struct iovec  iov[2] = { {&x, sizeof x}, {(void *) (intptr_t) buf, size} };
    struct msghdr msg = { 0, 0, iov, 1 + (buf && size), &ctl, sizeof ctl, 0 };
    int           ret;

    do ret = sendmsg(skt, &msg, 0);
    while (ret < 0 && errno == EINTR);

    return ret - sizeof(x) * (ret > 0);
}

static struct { int lvl, opt; } opts[SOCK_OPTS] = {
      { SOL_SOCKET,  SO_KEEPALIVE }
    , { IPPROTO_TCP, TCP_NODELAY  }
    , { F_GETFL,     O_NONBLOCK   }
    , { SOL_SOCKET,  SO_RCVBUF    }
    , { SOL_SOCKET,  SO_SNDBUF    }
    , { SOL_SOCKET,  SO_ERROR     } // sock_getopt only
    , { SOL_SOCKET,  SO_LINGER    } // placeholder
};

int
sock_getopt(int skt, SOCK_OPT opt)
{
    dyninit();

    int         ret, val;
    socklen_t   len;

    if (opt >= SOCK_OPTS) return errno = EINVAL, -1;

    if (opt == LINGER) {
        struct linger lng;
        len = sizeof lng;
        ret = getsockopt(skt, SOL_SOCKET, SO_LINGER, &lng, &len);
        return ret ? ret : lng.l_onoff ? lng.l_linger : 0;
    } else if (opts[opt].lvl == F_GETFL) {
        return !!(fcntl(skt, F_GETFL, 0) & opts[opt].opt);
    } else {
        len = sizeof val;
        ret = getsockopt(skt, opts[opt].lvl, opts[opt].opt, &val, &len);
        return ret ? ret : val;
    }
}

int
sock_setopt(int skt, SOCK_OPT opt, int val)
{
    dyninit();

    if (opt >= SOCK_OPTS) return errno = EINVAL, -1;

    struct linger lng = { val > 0, val }; //XXX why winwarn?
    return opt == LINGER ?
            setsockopt(skt, SOL_SOCKET, SO_LINGER, &lng, sizeof lng)
        : opts[opt].lvl == F_GETFL ?
            fcntl(skt, F_SETFL, BIT(fcntl(skt, F_GETFL, 0), opts[opt].opt, val))
        :   setsockopt(skt, opts[opt].lvl, opts[opt].opt, &val, sizeof val);
}

int
sock_addr(int skt, IPSTR ip, int *pport, char *name, int size)
{
    dyninit();

    INxADDR sin;
    socklen_t len = sizeof(sin);
    if (0 > getsockname(skt, (SOADDR*)&sin, &len))
        return -1;

    ininfo(&sin, ip, pport);
    return -(name && size
             && getnameinfo((SOADDR*) &sin, len, name, size, NULL, 0, 0));
}

int
sock_peer(int skt, IPSTR ip, int *pport, char *name, int size)
{
    dyninit();

    INxADDR sin;
    socklen_t len = sizeof(sin);
    if (0 > getpeername(skt, (SOADDR*)&sin, &len))
        return -1;

    ininfo(&sin, ip, pport);
    return -(name && size &&
             getnameinfo((SOADDR*)&sin, len, name, size, NULL, 0,
                          NI_NAMEREQD + NI_NOFQDN));
}

#ifdef SO_ORIGINAL_DST
int
sock_dest(int skt, IPSTR ip, int *pport)
{
    dyninit();

    INxADDR sin;
    socklen_t len = sizeof sin;
    if (0 > getsockopt(skt, SOL_IP, SO_ORIGINAL_DST, (SOADDR*)&sin, &len))
        return -1;

    ininfo(&sin, ip, pport);
    return 0;
}
#endif
//--------------|-------|-------------------------------------
int
udp_open(IPSTR const ip, int port)
{
    dyninit();

    if (port & -65536) return errno = EINVAL, -1;

    INxADDR addr;
    int addrlen = ininit(&addr, ip, port);
    if (addrlen < 0) return addrlen;

    int fd = socket(addr.inx_family, SOCK_DGRAM | sock_cloexec, IPPROTO_UDP);
    return fd >= 0 && ((!sock_cloexec && fcntl(fd, F_SETFD, FD_CLOEXEC)) ||
                       (port && bind(fd, (SOADDR*)&addr, addrlen))
        )? eclose(fd) : fd;
}

int
udp_recv(int fd, char *buf, int size, IPSTR ip, int *port)
{
    dyninit();

    INxADDR     addr;
    socklen_t   addrlen = sizeof(addr);
    int         len;

    do len = recvfrom(fd, buf, size, /*flags*/0, (SOADDR*)&addr, &addrlen);
    while (len < 0 && errno == EINTR);
    if (len >= 0) ininfo(&addr, ip, port);
    return len;
}

int
udp_send(int fd, char const *buf, int size, IPSTR const ip, int port)
{
    dyninit();

    INxADDR addr;
    int ret, addrlen = ininit(&addr, ip, port);
    do ret = sendto(fd, buf, size, /*flags*/0, (SOADDR const*) &addr, addrlen);
    while (ret < 0 && errno == EINTR);

    return ret;
}

//--------------|-------|-------------------------------------
// Code may be compiled on a machine supporting SOCK_CLOEXEC,
// then run on one that doesn't. glibc mocks accept4,
// except if you call it on a system whose kernel doesn't support it,
// accept4 reports ENOSYS.

static void
dyninit(void)
{
    if (sock_cloexec == 1) {    // initial state, ie neither 0 nor SOCK_CLOEXEC
#       ifdef SOCK_CLOEXEC
        sock_cloexec = SOCK_CLOEXEC;    // Safely re-enter dyninit from sock_bind
        int fd = sock_bind("", 0);
        if (fd < 0) {
            sock_cloexec = 0
            cmsg_cloexec = 0;
        } else {
#           ifdef HAS_ACCEPT4
            // Test whether glibc.accept4 really works:
            IN4ADDR sin;
            socklen_t len = sizeof(sin);
            listen(fd, 1);
            getsockname(fd, (SOADDR*)&sin, &len);
            int con = sock_connect("127.0.0.1", ntohs(sin.sin_port), 1);
            int ret = accept4(fd, NULL, NULL, SOCK_CLOEXEC);
            has_accept4 = ret >= 0 && fcntl(ret, F_GETFD, 0) & FD_CLOEXEC;
            close(ret);
            close(con);
#           endif
            close(fd);
        }
#       else
        sock_cloexec = 0;
#       endif
    }
    errno = 0;
}

static int
sockit(int domain)
{
    int fd = socket(domain, SOCK_STREAM | sock_cloexec, 0);
    return fd >= 0 && !sock_cloexec && fcntl(fd, F_SETFD, FD_CLOEXEC)
            ? eclose(fd) : fd;
}

static int
uninit(UNADDR * uap, char const *path)
{
    int len = strlen(path);
    if (len > (int) sizeof uap->sun_path) return 0;

    memset(uap, 0, sizeof(UNADDR));
#   ifdef __FreeBSD__
    uap->sun_len = len;
#   endif
    uap->sun_family = AF_UNIX;
    memcpy(uap->sun_path, path, len);
    return SUN_LEN(uap);
}

static void
ininfo(INxADDR const*xp, IPSTR ip, int *pport)
{
    if (pport) *pport = ntohs(xp->inx_port);
    if (!ip) return;
    if (xp->inx_family == AF_INET)
        inet_ntop(AF_INET,  (void const*)&xp->in4.sin_addr,  ip, sizeof(IPSTR));
    else
        inet_ntop(AF_INET6, (void const*)&xp->in6.sin6_addr, ip, sizeof(IPSTR));
}

static int
ininit(INxADDR *xp, IPSTR const ip, int port)
{
#   ifdef __FreeBSD__
#       define SETLEN(x) (int)(xp->in4.sin_len = (x))
#   else
#       define SETLEN(x) (int)(x)
#   endif
    // We rely on INADDR_ANY == 0x00000000
    memset(xp, 0, sizeof(*xp));
    xp->inx_port   = (uint16_t)htons((uint16_t)port);
    xp->inx_family = AF_INET;
    return !ip || !*ip ||
              1 == inet_pton(AF_INET, ip, &xp->in4.sin_addr)
            ? SETLEN(sizeof(IN4ADDR))
            : 1 == inet_pton(xp->inx_family = AF_INET6, ip, &xp->in6.sin6_addr)
            ? SETLEN(sizeof(IN6ADDR))
            : -1;
#   undef SETLEN
}
