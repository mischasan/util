#ifndef SOCK_H
#define SOCK_H

#ifndef ENTER_C
#ifdef __cplusplus
#   define ENTER_C extern "C" {
#   define LEAVE_C   };
#else
#   define ENTER_C
#   define LEAVE_C
#endif
#endif

ENTER_C

#define WAIT_FOREVER (-1)   // sock_ready(*,*,timeout)

// For (sock_addr,sock_dest,sock_peer), any of (ip,pport,name) may be NULL
// host="" is equivalent to host="localhost"
// host_ip() is needed for udp_send only.

typedef enum {
      SOCK_KEEPALIVE // TCP keepalive
    , SOCK_NODELAY   // Nonbuffered TCP, for tty-like responsiveness
    , SOCK_NOWAIT    // nonblocking I/O
    , SOCK_RCVSIZE   // (size) buffer size
    , SOCK_SNDSIZE   // (size) buffer size
    , SOCK_CONERR    // connection state (0 == connected).
    , SOCK_LINGER    // (secs) bg delay to flush output after close
    , SOCK_CORK      // Forced buffering
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

// Retrieve local socket address info.
int sock_addr(int skt, IPSTR ip, int *pport, char *name, int size);

int sock_bind(IPSTR const ip, int port);
void sock_close(int skt);
int sock_connect(char const *host, int port, int nowait);

// Create a UNIX socket (aka named pipe)
int sock_create(char const *path);

#ifdef linux
// Retrieve original destination address, before NAT redirect.
int sock_dest(int skt, IPSTR ip, int *pport);
#endif

int sock_getopt(int skt, SOCK_OPT);
int sock_open(char const *path);

// Retrieve peer address info.
int sock_peer(int skt, IPSTR ip, int *pport, char *name, int size);

// Wait for (or test, if waitsecs=0) socket to be ready.
//  mode=0:read,accept mode=1:write,connect,
//  Returns: -2:skt error, -1:select error, 0:timeout 1:ready
int sock_ready(int skt, int mode, int waitsecs);

int sock_recv(int skt, char *buf, int size);

// Receive fd and message via Unix socket:
int sock_recvfd(int skt, int *pfd, char *buf, int size);

int sock_send(int skt, char const*buf, int size);

// Send fd and message via Unix socket:
int sock_sendfd(int skt, int fd, char const*buf, int size);

int sock_setopt(int skt, SOCK_OPT, int val);

// Test (input pending | output okay | error) status for a list of sockets.
//  Each element of statusv[] is the OR of applicable enum values.
//  Returns: same as sock_ready, except ret > 0 is the number of sockets
//  with non-zero status.

enum { SOCK_CAN_RECV = 1, SOCK_CAN_SEND = 2, SOCK_IN_ERROR = 4 };
int sock_status(int nsocks, int const sockv[], int statusv[], int waitsecs);

int udp_open(IPSTR const ip, int port);
int udp_recv(int fd, char *buf, int size, IPSTR ip, int *port);
int udp_send(int fd, char const *buf, int size, IPSTR const ip, int port);

LEAVE_C

#endif//SOCK_H
