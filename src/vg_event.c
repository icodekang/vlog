#define _GNU_SOURCE

#include "vg_fmacros.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "vg_defs.h"
#include "vg_event.h"

void vlog_event_profile(vlog_event_t * a_event, int flag)
{
	vg_assert(a_event,);
	vg_profile(flag, "---event[%p][%s,%s][%s(%ld),%s(%ld),%ld,%d][%p,%s][%ld,%ld][%ld,%ld][%d]---",
			a_event,
			a_event->category_name, a_event->host_name,
			a_event->file, a_event->file_len,
			a_event->func, a_event->func_len,
			a_event->line, a_event->level,
			a_event->hex_buf, a_event->str_format,
			a_event->time_stamp.tv_sec, a_event->time_stamp.tv_usec,
			(long)a_event->pid, (long)a_event->tid,
			a_event->time_cache_count);
	return;
}

/*******************************************************************************/

void vlog_event_del(vlog_event_t * a_event)
{
	vg_assert(a_event,);
	if (a_event->time_caches) free(a_event->time_caches);
	vg_debug("vlog_event_del[%p]", a_event);
    free(a_event);
	return;
}

vlog_event_t *vlog_event_new(int time_cache_count)
{
	vlog_event_t *a_event;

	a_event = calloc(1, sizeof(vlog_event_t));
	if (!a_event) {
		vg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_event->time_caches = calloc(time_cache_count, sizeof(vlog_time_cache_t));
	if (!a_event->time_caches) {
		vg_error("calloc fail, errno[%d]", errno);
		free(a_event);
		return NULL;
	}
	a_event->time_cache_count = time_cache_count;

	/*
	 * at the vlog_init we gethostname,
	 * u don't always change your hostname, eh?
	 */
	if (gethostname(a_event->host_name, sizeof(a_event->host_name) - 1)) {
		vg_error("gethostname fail, errno[%d]", errno);
		goto err;
	}

	a_event->host_name_len = strlen(a_event->host_name);

	/* tid is bound to a_event
	 * as in whole lifecycle event persists
	 * even fork to oth pid, tid not change
	 */
	a_event->tid = pthread_self();

	a_event->tid_str_len = sprintf(a_event->tid_str, "%lu", (unsigned long)a_event->tid);
	a_event->tid_hex_str_len = sprintf(a_event->tid_hex_str, "%x", (unsigned int)a_event->tid);

#ifdef __linux__
	a_event->ktid = syscall(SYS_gettid);
#elif __APPLE__
    uint64_t tid64;
    pthread_threadid_np(NULL, &tid64);
    a_event->tid = (pid_t)tid64;
#endif

#if defined __linux__ || __APPLE__
	a_event->ktid_str_len = sprintf(a_event->ktid_str, "%u", (unsigned int)a_event->ktid);
#endif

	//vlog_event_profile(a_event, vg_DEBUG);
	return a_event;
err:
	vlog_event_del(a_event);
	return NULL;
}

/*******************************************************************************/
void vlog_event_set_fmt(vlog_event_t * a_event,
			char *category_name, size_t category_name_len,
			const char *file, size_t file_len, const char *func, size_t func_len,  long line, int level,
			const char *str_format, va_list str_args)
{
	/*
	 * category_name point to vlog_category_output's category.name
	 */
	a_event->category_name = category_name;
	a_event->category_name_len = category_name_len;

	a_event->file = (char *) file;
	a_event->file_len = file_len;
	a_event->func = (char *) func;
	a_event->func_len = func_len;
	a_event->line = line;
	a_event->level = level;

	a_event->generate_cmd = vlog_FMT;
	a_event->str_format = str_format;
	va_copy(a_event->str_args, str_args);

	/* pid should fetch eveytime, as no one knows,
	 * when does user fork his process
	 * so clean here, and fetch at spec.c
	 */
	a_event->pid = (pid_t) 0;

	/* in a event's life cycle, time will be get when spec need,
	 * and keep unchange though all event's life cycle
	 * vlog_spec_write_time gettimeofday
	 */
	a_event->time_stamp.tv_sec = 0;
	return;
}

void vlog_event_set_hex(vlog_event_t * a_event,
			char *category_name, size_t category_name_len,
			const char *file, size_t file_len, const char *func, size_t func_len,  long line, int level,
			const void *hex_buf, size_t hex_buf_len)
{
	/*
	 * category_name point to vlog_category_output's category.name
	 */
	a_event->category_name = category_name;
	a_event->category_name_len = category_name_len;

	a_event->file = (char *) file;
	a_event->file_len = file_len;
	a_event->func = (char *) func;
	a_event->func_len = func_len;
	a_event->line = line;
	a_event->level = level;

	a_event->generate_cmd = vlog_HEX;
	a_event->hex_buf = hex_buf;
	a_event->hex_buf_len = hex_buf_len;

	/* pid should fetch eveytime, as no one knows,
	 * when does user fork his process
	 * so clean here, and fetch at spec.c
	 */
	a_event->pid = (pid_t) 0;

	/* in a event's life cycle, time will be get when spec need,
	 * and keep unchange though all event's life cycle
	 */
	a_event->time_stamp.tv_sec = 0;
	return;
}
