#ifndef __vlog_conf_h
#define __vlog_conf_h

#include "vg_defs.h"
#include "vg_format.h"
#include "vg_rotater.h"

typedef struct vlog_conf_s {
	char file[MAXLEN_PATH + 1];
	char cfg_ptr[MAXLEN_CFG_LINE*MAXLINES_NO];
	char mtime[20 + 1];

	int strict_init;
	size_t buf_size_min;
	size_t buf_size_max;

	char rotate_lock_file[MAXLEN_CFG_LINE + 1];
	vlog_rotater_t *rotater;

	char default_format_line[MAXLEN_CFG_LINE + 1];
	vlog_format_t *default_format;

	unsigned int file_perms;
	size_t fsync_period;
	size_t reload_conf_period;

	vg_arraylist_t *levels;
	vg_arraylist_t *formats;
	vg_arraylist_t *rules;
	int time_cache_count;
} vlog_conf_t;

extern vlog_conf_t * vlog_env_conf;

vlog_conf_t *vlog_conf_new(const char *config);
void vlog_conf_del(vlog_conf_t * a_conf);
void vlog_conf_profile(vlog_conf_t * a_conf, int flag);

#endif
