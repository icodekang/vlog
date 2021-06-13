#ifndef __vlog_format_h
#define __vlog_format_h

#include "vg_thread.h"
#include "vg_defs.h"

typedef struct vlog_format_s vlog_format_t;

struct vlog_format_s {
	char name[MAXLEN_CFG_LINE + 1];	
	char pattern[MAXLEN_CFG_LINE + 1];
	vg_arraylist_t *pattern_specs;
};

vlog_format_t *vlog_format_new(char *line, int * time_cache_count);
void vlog_format_del(vlog_format_t * a_format);
void vlog_format_profile(vlog_format_t * a_format, int flag);

int vlog_format_gen_msg(vlog_format_t * a_format, vlog_thread_t * a_thread);

#define vlog_format_has_name(a_format, fname) \
	STRCMP(a_format->name, ==, fname)

#endif
