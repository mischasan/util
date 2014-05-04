#ifndef TDSSOCK_H
#define TDSSOCK_H

#define WAIT_FOREVER (-1)   // sock_ready(*,*,timeout)

typedef enum {
      KEEPALIVE // TCP keepalive
    , NODELAY   // Nonbuffered TCP, for tty-like responsiveness
    , NOWAIT    // nonblocking I/O
    , RCVSIZE   // (size) buffer size
    , SNDSIZE   // (size) buffer size
    , CONERR    // connection state (0 == connected).
    , LINGER    // (secs) bg delay to flush output after close
    , SOCK_OPTS
} SOCK_OPT;

#ifndef INET6_ADDRSTRLEN
#   define INET6_ADDRSTRLEN 46  // netinet/in.h
#endif
typedef char IPSTR[INET6_ADDRSTRLEN];

// 123456789.123456789.123456789.123456789.123456789.
// xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx\0        - IPv6 max
// 0000:0000:0000:0000:0000:FFFF:ddd.ddd.ddd.ddd\0  - IPv4 over IPv6

int host_ip(char const *host, int port/*optional*/, IPSTR ip);

int sock_accept(int skt);
int sock_addr(int skt, IPSTR ip, int *pport, char *name, int size);
int sock_bind(IPSTR const ip, int port);
void sock_close(int skt);
int sock_connect(char const *host, int port, int nowait);
int sock_create(char const *path);
#ifdef linux
int sock_dest(int skt, IPSTR ip, int *pport);
#endif
int sock_getopt(int skt, SOCK_OPT);
int sock_open(char const *path);
int sock_peer(int skt, IPSTR ip, int *pport, char *name, int size);
int sock_ready(int skt, int mode, int waitsecs);
int sock_recv(int skt, char *buf, int size);
int sock_recvfd(int skt, int *pfd, char *buf, int size);
int sock_send(int skt, char const*buf, int size);
int sock_sendfd(int skt, int fd, char const*buf, int size);
int sock_setopt(int skt, SOCK_OPT, int val);

int udp_open(IPSTR const ip, int port);
int udp_recv(int fd, char *buf, int size, IPSTR ip, int *port);
int udp_send(int fd, char const *buf, int size, IPSTR const ip, int port);

#endif//TDSSOCK_H
