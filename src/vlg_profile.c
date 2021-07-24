#include "vlg_fmacros.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "vlg_profile.h"
#include "vlg_xplatform.h"

static void vlg_time(char *time_str, size_t time_str_size)
{
	time_t tt;
	struct tm local_time;

	time(&tt);
	localtime_r(&tt, &local_time);
	strftime(time_str, time_str_size, "%m-%d %T", &local_time);

	return;
}

int vlg_profile_inner(int flag, const char *file, const long line, const char *fmt, ...)
{
	va_list args;
	char time_str[20 + 1];
	FILE *fp = NULL;

	static char *debug_log = NULL;
	static char *error_log = NULL;
	static size_t init_flag = 0;

	if (!init_flag) {
		init_flag = 1;
		debug_log = getenv("VLOG_DEBUG");
		error_log = getenv("VLOG_ERROR");
	}

	switch (flag) {
	case vlg_DEBUG:
 		if (debug_log == NULL) return 0;
		fp = fopen(debug_log, "a");
		if (!fp) return -1;
		vlg_time(time_str, sizeof(time_str));
		fprintf(fp, "%s DEBUG (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	case vlg_WARN:
 		if (error_log == NULL) return 0;
		fp = fopen(error_log, "a");
		if (!fp) return -1;
		vlg_time(time_str, sizeof(time_str));
		fprintf(fp, "%s WARN  (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	case vlg_ERROR:
 		if (error_log == NULL) return 0;
		fp = fopen(error_log, "a");
		if (!fp) return -1;
		vlg_time(time_str, sizeof(time_str));
		fprintf(fp, "%s ERROR (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	}

	/* writing file twice(time & msg) is not atomic
	 * may cause cross
	 * but avoid log size limit */
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fprintf(fp, "\n");

	fclose(fp);
	return 0;
}

