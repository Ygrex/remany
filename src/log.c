#include "log.h"

/* {{{ output(…) - output formatted message to stderr */
static char *output(const char *prefix, char *msg) {
	char ts[512];
	struct tm *tr = NULL;
	time_t t = time(NULL);
	if (t == (time_t)(-1)) {
		perror("output(): time()");
	} else {
		tr = localtime(&t);
		if (tr == NULL) {
			perror("output(): localtime()");
		}
	}
	fputs(prefix, stderr);
	fputs(": ", stderr);
	if (tr != NULL) {
		ts[
			strftime(
				ts,
				sizeof(ts) / sizeof(ts[0]) - 1,
				"%F %T %Z",
				tr
			)
		] = '\0';
		ts[sizeof(ts) / sizeof(ts[0]) - 1] = '\0';
		fputs("[", stderr);
		fputs(ts, stderr);
		fputs("] ", stderr);
	}
	fputs(msg, stderr);
	fputs("\n", stderr);
	fflush(stderr);
	return msg;
}
/* }}} output() */

/* {{{ warn(…) - output warning */
char *warn(char *msg) { return output("WW", msg) ; }
/* }}} warn() */

/* {{{ warn(…) - output warning */
char *debug(char *msg) { return output("DD", msg) ; }
/* }}} warn() */

