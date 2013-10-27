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

/* concurs [playfile]: monitor processes that use "concur".
 *
 *     Participating processes have the environment variable "CONCUR"
 *    set to "name[:host]", where "name" identifies the process and
 *    host is the IP address of the "concurs" monitor.
 *
 *    Each participant starts by sending its name to the monitor.
 *    The monitor responds with its mode (record or play).
 *    Thereafter, each participant sends "event" strings;
 *    in play mode, the participant blocks waiting for a 1-byte response.
 *
 *    Given a playfile, concurs responds to each blocking request,
 *    when it reads the participant's name and event from its playfile.
 *
 * FILES
 *    playfile - output of "concurs", which can be edited (with care).
 *
 *	Lines beginning with '#:' are ignored.
 *	All other lines beginning with '#' are echoed to stdout.
 *	All other lines are expected to be script lines of the form:
 *	<name>:<event comment>
 *	... <name> is a process identifier
 *	... <event comment> is the string sent to "concurs".
 *
 *    playfile may be a named pipe, allowing dynamic control
 *    of the sequence of events.
 *TODO
 * - add "-p <port>" and "-t <timeout>" options.
 * - if "playfile" is a socket name, reopen it every time it closes.
 *  
 */

#include "msutil.h"
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>

#include "_concur.h"

static void     play(int skt, const char *file, int timeout);
static void     record(int skt);

static    int	max_fd;
static    fd_set	open_fds;
static    char	*playerv[FD_SETSIZE];

int
main(int argc, char **argv)
{
    int		timeout = argc > 2 ? atoi(argv[2]) : 0;
    char *const	env_port = getenv("CONCUR_PORT");
    int		port     = env_port ? atoi(env_port) : CONCUR_PORT;
    int		skt	 = sock_bind("", port);

    if (skt < 0) {
	perror("concurs");
	exit(1);
    }

    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc == 1) record(skt); 
    else play(skt, argv[1], timeout);

    return    0;
}

static void
record(int skt)
{
    char	mode[] = { RECORD };

    max_fd     = skt;
    FD_ZERO(&open_fds);
    FD_SET(skt, &open_fds);

    while (1) {
	int    fd, nfds;
	fd_set    read_fds = open_fds, error_fds = open_fds;
	char    buf[MAX_CONCUR_MSG];

	if (0 > (nfds = select(max_fd+1, &read_fds, NULL, &error_fds, 0))) {
	    if (errno == EINTR)
		continue;
	    fprintf(stderr, "# concurs: select failed: %s\n", strerror(errno));
	    exit(1);
	}

	if (FD_ISSET(skt, &read_fds)) {
	    --nfds;
	    fd = sock_accept(skt);

	    if (0 >= sock_recv(fd, buf, sizeof buf)) {
		fprintf(stderr, "concurs: recv new player failed: %s\n", strerror(errno));
	    } else {
		sock_send(fd, mode, sizeof(mode));
		playerv[fd] = strdup(buf);
		printf("%s:%s\n", buf, buf);
		FD_SET(fd, &open_fds);
		if (max_fd < fd) max_fd = fd;
	    }
	}

	for (fd = skt+1; nfds > 0; ++fd) {
	    int    endit = 0, ret;

	    if (FD_ISSET(fd, &read_fds)) {
		--nfds;
		ret = sock_recv(fd, buf, sizeof(buf));
		if (ret < 0)
		    fprintf(stderr, "concurs: error reading %s\n", playerv[fd]);
		if (ret > 0)
		    printf("%s:%.*s\n", playerv[fd], ret, buf);
		else    endit = 1;
	    }

	    if (FD_ISSET(fd, &error_fds)) {
		--nfds;
		endit = 1;
		fprintf(stderr, "concurs: error from %s\n", playerv[fd]);
	    }

	    if (endit) {
		close(fd);
		FD_CLR(fd, &open_fds);
		free(playerv[fd]);
		playerv[fd] = NULL;
		while (max_fd > skt && !playerv[max_fd])
		    --max_fd;
	    }
	}
    }
}

static void
play(int skt, const char *file, int timeout)
{
    char	mode[] = { PLAY };
    char	*cp;
    char	expect[MAX_CONCUR_NAME+1+MAX_CONCUR_MSG];
    char	actual[MAX_CONCUR_MSG];
    MAP	conh = map_create((map_diff*)strcmp, (map_hash*)fnv04, NULL);
    FILE	*fp = fopen(file, "r");
    fd_set	fds;

    if (!fp) {
	fprintf(stderr, "error: cannot read %s\n", file);
	exit(2);
    }

    FD_ZERO(&fds);

    while (fgets(expect, sizeof expect, fp)) {
	int	fd, ret;
	struct timeval    to = { timeout, 0 };

	for (cp = expect + strlen(expect); cp > expect && isspace(cp[-1]); *--cp = 0);

	if (!expect[0])
	    continue;

	if (expect[0] == '#') {
	    if (expect[1] != ':') 
		fputs(expect, stdout);
	    continue;
	}

	if (!(cp = strchr(expect, ':'))) {
	    fprintf(stderr, "concurs: invalid script line:\n%s\n", expect);
	    continue;
	}
	*cp++ = 0;

	while (!(fd = (int)(uintptr_t)map_get(conh, expect))) {
	    FD_SET(skt, &fds);
	    if (timeout && 0 > select(skt+1, &fds, NULL, NULL, &to)) {
		fprintf(stderr, "concurs: timed out waiting for %s:connect\n", expect);
		exit(1);
	    }
	    FD_CLR(skt, &fds);
		
	    fd = sock_accept(skt);
	    FD_SET(fd, &open_fds);
	    cp = playerv[fd] = strdup(expect);
	    map_set(conh, playerv[fd], (void*)(uintptr_t)fd);
	}

	FD_SET(fd, &fds);
	if (timeout && 0 > select(skt+1, &fds, NULL, NULL, &to)) {
	    fprintf(stderr, "concurs: error waiting for %s:%s\n", expect, cp);
	    exit(1);
	}
	FD_CLR(fd, &fds);

	if (0 >= (ret = sock_recv(fd, actual, sizeof actual))) {
	    close(fd);
	    FD_CLR(fd, &open_fds);
	    map_del(conh, playerv[fd]);
	    free(playerv[fd]);
	    playerv[fd] = NULL;
	}

	sock_send(fd, mode, sizeof(mode));
	//REVISIT: select(nowait) on all open fds and report who was blocked.
	printf("%s:%s\n", expect, actual);
	if (strcmp(cp, actual)) {
	    fprintf(stderr, "concurs: broken\n");
	    exit(1);
	}
    }
}
