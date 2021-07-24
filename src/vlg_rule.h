#ifndef __vlog_rule_h
#define __vlog_rule_h

#include <stdio.h>
#include <pthread.h>

#include "vlg_defs.h"
#include "vlg_format.h"
#include "vlg_thread.h"
#include "vlg_rotater.h"
#include "vlg_record.h"

typedef struct vlog_rule_s vlog_rule_t;

typedef int (*vlog_rule_output_fn) (vlog_rule_t * a_rule, vlog_thread_t * a_thread);

struct vlog_rule_s {
	char category[MAXLEN_CFG_LINE + 1];
	char compare_char;
	/* 
	 * [*] log all level
	 * [.] log level >= rule level, default
	 * [=] log level == rule level 
	 * [!] log level != rule level
	 */
	int level;
	unsigned char level_bitmap[32]; /* for category determine whether ouput or not */

	unsigned int file_perms;
	int file_open_flags;

	char file_path[MAXLEN_PATH + 1];
	vlg_arraylist_t *dynamic_specs;
	int static_fd;
	dev_t static_dev;
	ino_t static_ino;

	long archive_max_size;
	int archive_max_count;
	char archive_path[MAXLEN_PATH + 1];
	vlg_arraylist_t *archive_specs;

	FILE *pipe_fp;
	int pipe_fd;

	size_t fsync_period;
	size_t fsync_count;

	vlg_arraylist_t *levels;
	int syslog_facility;

	vlog_format_t *format;
	vlog_rule_output_fn output;

	char record_name[MAXLEN_PATH + 1];
	char record_path[MAXLEN_PATH + 1];
	vlog_record_fn record_func;
};

vlog_rule_t *vlog_rule_new(char * line,
		vlg_arraylist_t * levels,
		vlog_format_t * default_format,
		vlg_arraylist_t * formats,
		unsigned int file_perms,
		size_t fsync_period,
		int * time_cache_count);

void vlog_rule_del(vlog_rule_t * a_rule);
void vlog_rule_profile(vlog_rule_t * a_rule, int flag);
int vlog_rule_match_category(vlog_rule_t * a_rule, char *category);
int vlog_rule_is_wastebin(vlog_rule_t * a_rule);
int vlog_rule_set_record(vlog_rule_t * a_rule, vlg_hashtable_t *records);
int vlog_rule_output(vlog_rule_t * a_rule, vlog_thread_t * a_thread);

#endif
