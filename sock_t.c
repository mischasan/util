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

#if 0
XXX add test cases:
- all async ops! (esp connect).
- sock_create where name is in use (not removable)
- " " where filename is improper >106 chars?
- call sock_open/sock_connect/sock_send/sock_sendfd not in child process (so gcov sees it)
    The problem is that tap requires child processes to _exit()
- EINTR handling works.
- sock_????fd on INET sockets

//XXX add functionality: send/recv BUFVEC's
#endif
#include "msutil.h"
#include "sock.h"
#include <errno.h>
#include <fcntl.h>      // open
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h> // SOCK_CLOEXEC
#include "tap.h"

//#if !defined(__BSD_VISIBLE)
//    typedef __sighandler_t sig_t;
//#endif
// This isn't right, but better than nothing.
//#ifdef __x86_64
//#   define stat  stat64
//#   define fstat fstat64
//#endif
int     secs = 1;    // to override when debugging.
int     pid;

static void on_alarm(void) { kill(pid, SIGTERM); exit(2); }

int main(int argc, char **argv)
{
    plan_tests(36);
    char    usock[] = "sock_t.u";

    int         port = argc > 1 ? atoi(argv[1]) : 9999;
    // This cannot be "localhost", which can map to IPv6 or IPv4
    // (order of returned results by getaddrinfo not deterministic).
    char const  *host = argc > 2 ? argv[2] : "127.0.0.1";
    setvbuf(stdout, NULL, _IOLBF, 0);

    IPSTR   ip = "", srvip;
    ok(host_ip("nobody.home", 80, ip), "unknown hostname");
    if (ok(!host_ip("google.ca", 80, ip), "google.ca"))
        fprintf(stderr, "# ip = %s\n", ip);

    int     rc, x, c, s = sock_bind(0, port);
    if (!ok(s >= 0, "sock_bind(%d)", port)) die("bind:");

    int         u = sock_create(usock);
    if (!ok(u >= 0, "sock_create(%s)", usock)) die("create:");

    rc = sock_getopt(s, SOCK_KEEPALIVE);
    ok(rc == 0, "getopt SOCK_KEEPALIVE: %d %s", rc, errname[errno]);

    rc = sock_setopt(s, SOCK_KEEPALIVE, 1);
    ok(rc == 0, "setopt SOCK_KEEPALIVE=1: %d %s", rc, errname[errno]);

    int      opt = sock_getopt(s, SOCK_LINGER);
    ok(opt == 0, "getopt SOCK_LINGER: %d %s", opt, errname[errno]);

    rc = sock_setopt(s, SOCK_LINGER, 8);
    ok(rc == 0, "setopt SOCK_LINGER=8 (secs): %d %s", rc, errname[errno]);

    rc = sock_getopt(s, SOCK_LINGER);
    ok(rc == 8, "getopt SOCK_LINGER: %d", rc);

    rc = sock_setopt(s, SOCK_LINGER, opt);
    ok(rc == 0, "setopt SOCK_LINGER=%d: %d %s", opt, rc, errname[errno]);

    rc = sock_setopt(s, SOCK_NOWAIT, 1);
    ok(!rc, "setopt SOCK_NOWAIT=1: %d", rc);

    rc = sock_getopt(s, SOCK_NOWAIT);
    ok(rc != 0, "getopt SOCK_NOWAIT: %d", rc);

    rc = sock_setopt(u, SOCK_NOWAIT, 1);
    ok(rc == 0, "setopt(unix, SOCK_NOWAIT=1): %d", rc);

    alarm(secs);

    x = sock_accept(s);
    ok(x == -1 && errno == EAGAIN, "accept(inet) did not block: %d (%s)", x, errname[errno]);

    x = sock_accept(u);
    ok(x == -1 && errno == EAGAIN, "accept(unix) did not block: %d (%s)", x, errname[errno]);

    alarm(0);

    sock_setopt(s, SOCK_NOWAIT, 0);
    sock_setopt(u, SOCK_NOWAIT, 0);

    rc = sock_getopt(s, SOCK_NOWAIT);
    ok(rc == 0, "getopt SOCK_NOWAIT: %d", rc);

    rc = sock_getopt(s, 666);
    ok(rc < 0, "getopt(bad_option) rejected: %s", errname[errno]);

    struct stat st;

    stat("/dev/null", &st);
    unsigned    devnull = st.st_ino;
    diag("inode(/dev/null) is %d", (int)devnull);

    stat("/dev/zero", &st);
    unsigned    devzero = st.st_ino;
    diag("inode(/dev/zero) is %d", (int)devzero);

    // Ensure that the test terminates even if a child goes bad.
    signal(SIGALRM, (sig_t)on_alarm);

    //--------------|-------|-------------------------------------
    if (!(pid = fork())) {
        close(s);
        close(u);

        int     fd1 = open("/dev/null", O_WRONLY);
        int     fd2 = open("/dev/zero", O_RDONLY);
        if (0 > (c = sock_connect(host, port, 0)))      die("[client]sock_connect(%s,%d):", host, port);
        if (0 > sock_peer(c, srvip, NULL, NULL, 0))     die("[client]sock_peer:");
        fprintf(stderr, "# [client] server ip=%s\n", srvip);
        if (0 > write(c, "hello", 5))                   die("[client]write(inet):");
        if (0 > (u = sock_open(usock)))                 die("[client]sock_open:");
        if (0 > sock_sendfd(u, fd1, "howdy,", 6))       die("[client]sendfd(1):");
        if (0 > sock_sendfd(u, fd2, "doody", 5))        die("[client]sendfd(2):");

        pause();    // Waiting for SIGTERM.
        _exit(0);   //NOTREACHED?
    }

    x = sock_accept(s);
    ok(x >= 0, "accept(inet): %d %s", x, errname[errno]);
    ok(fcntl(x, F_GETFD, 0) & FD_CLOEXEC, "accepted socket has CLOEXEC set");

    char    buf[99] = {};
    rc = sock_peer(x, ip, NULL, buf, sizeof(buf));
    ok(!rc, "peer addr=%s:%u, hostname=%s %s", ip, port, buf, errname[errno]);

    memset(buf, 0, sizeof(buf));
    rc = sock_recv(x, buf, sizeof(buf));
    ok(rc == 5 && !memcmp(buf, "hello", 5), "received: (%d) '%.6s'", rc, buf);

    close(x);
    close(s);

    kill(pid, SIGTERM);
    pid = wait(&rc);
    ok(WEXITSTATUS(rc) == 0,  "client exit status: %x", WEXITSTATUS(rc));

    //--------------|-------|-------------------------------------
    s = sock_bind("::", 0);
    if (s < 0 && errno == EAFNOSUPPORT) {
        skip(4, "Skipping tests requiring IPv6");
    } else {
        char    name[999] = "";
        rc = sock_addr(s, ip, &port, name, sizeof(name));
        ok(!rc, "sock_addr for ipv6 returns %d %s: ip=%s port=%d name=%s",
           rc, rc < -1 ? eainame[-rc] : errname[errno], ip, port, name);
        if (!(pid = fork())) {
            close(s);
            if (0 > (s = sock_connect(host, port, 0)))      die("[client6]sock_connect:");
            if (0 > sock_peer(s, srvip, NULL, NULL, 0))     die("[client6]sock_peer:");
            pause();
            _exit(0);
        }

        x = sock_accept(s);
        ok(x >= 0, "accept(inet6): %d %s", x, errname[errno]);
        memset(ip, 0, sizeof(ip));
        memset(buf, 0, sizeof(buf));
        rc = sock_peer(x, ip, NULL, buf, sizeof(buf));
        ok(!rc, "peer addr=%s:%u, hostname=%s %s", ip, port, buf, errname[errno]);

        kill(pid, SIGTERM);
        pid = wait(&rc);
        ok(WEXITSTATUS(rc) == 0,  "client exit status: %x", WEXITSTATUS(rc));
        close(x);
        close(s);
    }

    //--------------|-------|-------------------------------------
    x = sock_accept(u);
    ok(x >= 0, "accept(unix): %d %s", x, errname[errno]);

    memset(buf, 0, sizeof(buf));

    int     fd = -9;
    rc = sock_recvfd(x, &fd, buf, sizeof(buf));
    ok(rc == 6 && fd > 0, "recvfd returned %d, fd=%d", rc, fd);
    if (rc > 0) diag("data is '%.*s'", rc, buf);

    rc = fstat(fd, &st);
    ok(rc == 0 && st.st_ino == devnull, "fstat(%d): %d inode=%d", fd, rc, rc ? 0 : (int)st.st_ino);

    fd = -8;
    rc = sock_recvfd(x, &fd, buf+6, sizeof(buf));
    ok(rc == 5 && fd > 0, "recvfd returned %d, fd=%d", rc, fd);
    if (rc > 0) diag("data is '%.*s'", rc, buf+6);
    ok(!strcmp(buf, "howdy,doody"), "data is correct");

    rc = fstat(fd, &st);
    ok(rc == 0 && st.st_ino == devzero, "fstat(%d): %d inode=%d", fd, rc, rc ? 0 : (int)st.st_ino);

    alarm(secs);
    char    remhost[] = "aa.com";
    x = sock_connect(remhost, 80, SOCK_NOWAIT);
    ok(x >= 0 && errno == EINPROGRESS, "connect(%s,80,SOCK_NOWAIT) did not block: (%s)", remhost, errname[errno]);

    char    req[] = "GET / HTTP/1.0\r\n\r\n";
    rc = sock_send(x, req, strlen(req));
    ok(rc < 0, "send while connect is EINPROGRESS fails: %d (%s)", rc, errname[errno]);
    ok(sock_ready(x, /*WRITE|CONNECT*/1, /*SECS*/1), "connect ready in 1 sec");
    rc = sock_send(x, req, strlen(req));
    ok(rc > 0, "send now succeeds");

    unlink(usock);
#   undef  _
#   define _ "---------"
    x = sock_create(_"1"_"2"_"3"_"4"_"5"_"6"_"7"_"8"_"9"_"0"_"1");
    alarm(0);
    return exit_status();
}
