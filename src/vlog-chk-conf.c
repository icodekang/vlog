#include "vlg_fmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "vlog.h"
#include "vlg_version.h"


int main(int argc, char *argv[])
{
	int rc = 0;
	int op;
	int quiet = 0;
	static const char *help = 
		"usage: vlog-chk-conf [conf files]...\n"
		"\t-q,\tsuppress non-error message\n"
		"\t-h,\tshow help message\n"
		"vlog version: " vlog_VERSION "\n";

	while((op = getopt(argc, argv, "qhv")) > 0) {
		if (op == 'h') {
			fputs(help, stdout);
			return 0;
		} else if (op == 'q') {
			quiet = 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		fputs(help, stdout);
		return -1;
	}

	setenv("vlog_PROFILE_ERROR", "/dev/stderr", 1);
	setenv("vlog_CHECK_FORMAT_RULE", "1", 1);

	while (argc > 0) {
		rc = vlog_init(*argv);
		if (rc) {
			printf("\n---[%s] syntax error, see error message above\n",
				*argv);
			exit(2);
		} else {
			vlog_fini();
			if (!quiet) {
				printf("--[%s] syntax right\n", *argv);
			}
		}
		argc--;
		argv++;
	}

	exit(0);
}
