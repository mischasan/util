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
