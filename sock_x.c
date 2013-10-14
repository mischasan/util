// Compare TCP and UNIX sockets for throughput.
// This merrily leaks fds.
// Note that this is NOT a good simulation of
// interaction (process context switching).
#include "msutil.h"

#define BYTES   32768
#define COUNT   32768
char    SockPath[] = "sockit2me", buf[BYTES];
int     i;

static double pull(int fd) 
{ 
    double  t = tick();
    while (0 < read(fd, buf, BYTES));
    close(fd);
    return tick() - t;
}

static void push(int fd) 
{   
    for (i = COUNT; i-->0;) write(fd, buf, BYTES);
    close(fd);
    _exit(0);
}

int main(void)
{
    int     s, port;

    s = sock_bind(NULL, 0);             // (any i/f, any port)
    sock_addr(s, NULL, &port, NULL, 0);
    if (!fork()) push(sock_connect("127.0.0.1", port, 0));
    double  tt = pull(sock_accept(s));
    close(s);

    s = sock_create(SockPath);
    if (!fork()) push(sock_open(SockPath));
    double  tu = pull(sock_accept(s));
    close(s);

    // Compare to a std unix pipe (i.e. unix socket without the socket layer).
    int     p[2];
    pipe(p);
    if (!fork()) push(p[1]);
    close(p[1]);
    double  tp = pull(p[0]);

    fprintf(stderr, "%dMB tt=%.3f tu=%.3f tp=%.3f\n", BYTES/1024*COUNT/1024, tt, tu, tp);

    unlink(SockPath);
    return 0;
}
