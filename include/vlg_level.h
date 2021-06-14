#ifndef __vlog_level_h
#define __vlog_level_h

#include "vlg_defs.h"

typedef struct vlog_level_s {
	int int_level;
	char str_uppercase[MAXLEN_PATH + 1];
	char str_lowercase[MAXLEN_PATH + 1];
	size_t str_len;
       	int syslog_level;
} vlog_level_t;

vlog_level_t *vlog_level_new(char *line);
void vlog_level_del(vlog_level_t *a_level);
void vlog_level_profile(vlog_level_t *a_level, int flag);

#endif
