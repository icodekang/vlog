#ifndef __vlog_event_h
#define __vlog_event_h

#include <sys/types.h>  /* for pid_t */
#include <sys/time.h>   /* for struct timeval */
#include <pthread.h>    /* for pthread_t */
#include <stdarg.h>     /* for va_list */
#include "vg_defs.h"

typedef enum {
	vlog_FMT = 0,
	vlog_HEX = 1,
} vlog_event_cmd;

typedef struct vlog_time_cache_s {
	char str[MAXLEN_CFG_LINE + 1];
	size_t len;
	time_t sec;
} vlog_time_cache_t;

typedef struct {
	char *category_name;
	size_t category_name_len;
	char host_name[256 + 1];
	size_t host_name_len;

	const char *file;
	size_t file_len;
	const char *func;
	size_t func_len;
	long line;
	int level;

	const void *hex_buf;
	size_t hex_buf_len;
	const char *str_format;
	va_list str_args;
	vlog_event_cmd generate_cmd;

	struct timeval time_stamp;

	time_t time_local_sec;
	struct tm time_local;

	vlog_time_cache_t *time_caches;
	int time_cache_count;

	pid_t pid;
	pid_t last_pid;
	char pid_str[30 + 1];
	size_t pid_str_len;

	pthread_t tid;
	char tid_str[30 + 1];
	size_t tid_str_len;

	char tid_hex_str[30 + 1];
	size_t tid_hex_str_len;

#if defined __linux__ || __APPLE__
	pid_t ktid;
	char ktid_str[30+1];
	size_t ktid_str_len;
#endif
} vlog_event_t;


vlog_event_t *vlog_event_new(int time_cache_count);
void vlog_event_del(vlog_event_t * a_event);
void vlog_event_profile(vlog_event_t * a_event, int flag);

void vlog_event_set_fmt(vlog_event_t * a_event,
			char *category_name, size_t category_name_len,
			const char *file, size_t file_len, const char *func, size_t func_len, long line, int level,
			const char *str_format, va_list str_args);

void vlog_event_set_hex(vlog_event_t * a_event,
			char *category_name, size_t category_name_len,
			const char *file, size_t file_len, const char *func, size_t func_len, long line, int level,
			const void *hex_buf, size_t hex_buf_len);

#endif
