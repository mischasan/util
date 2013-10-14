#ifndef	_CONCUR_H_
#define	_CONCUR_H_

/* concur - synchronize concurrent processes.
 *	"concur" makes concurrent-process tests repeatable.
 *
 * When the "concurs" daemon is in "record" mode,
 * all processes log event strings to that daemon.
 *
 * In "play" mode, processes follow the same logic,
 * but each concur() call blocks until the concur daemon
 * indicates they can proceed.
 *
 * Example:
 *	Prior to each blocking call that may be affected by
 *	a peer process, a program makes a "concur" call.
 *
 * Environment
 *	CONCUR - <name>[:<host>]
 *		<name>		identifies the process.
 *		<host>		the "concurs" server (default: localhost).
 */

#define	CONCUR_PORT		5305	//qv Chime and iana.org

void concur(const char *fmt, ...);
void concur_exit(void);	 // optional: explicit reset to uninitialized state

#endif//_CONCUR_H_
