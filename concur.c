#include "msutil.h"
#include <stdarg.h>
#include <sys/socket.h>	    // send
#include "_concur.h"

static struct state_s { E_CONCUR mode; char *name; int conn; }
    state = { UNINIT, NULL, -1 };

void
concur(const char *fmt, ...)
{
    va_list     ap;
    char        *buf;
    int        len;

    if (!fmt || !*fmt) 
        return;

    if (state.mode == UNINIT) {
	const char  *host    = "127.0.0.1";
	char	    *cp	    = getenv("CONCUR");

	if (!cp || *cp == '#')
	    return;

	state.name  = strdup(cp);
	cp	    = strchr(state.name, ':');
	if (cp) {
	    *cp++ = 0;
	    host = cp;
	}

	if (0 <= (state.conn = sock_connect(host, CONCUR_PORT, 0))) {
	    state.mode = CONNECT;
	    concur(state.name);
	} else {
	    concur_exit();
	}
    }

    if (state.mode == OFF)
	return;

    va_start(ap, fmt);

    len = vasprintf(&buf, fmt, ap);
    if (0 > sock_send(state.conn, buf, len)
        || (state.mode == PLAY
	     && 0 > sock_recv(state.conn, buf, 1))) {

        fprintf(stderr, "concur terminates %s at %s\n", state.name, buf);
        exit(1);
    }

    if (state.mode == CONNECT)
        state.mode = buf[0];

    free(buf);
}

void
concur_exit()
{
    close(state.conn);    // may be invalid fd
    free(state.name);
    state = (struct state_s) { UNINIT, NULL, -1 };
}
