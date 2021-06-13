#ifndef __vlog_record_h
#define __vlog_record_h

#include "vg_defs.h"

/* record is user-defined output function and it's name from configure file */
typedef struct vlog_msg_s {
	char *buf;
	size_t len;
	char *path;
} vlog_msg_t; /* 3 of this first, see need thread or not later */

typedef int (*vlog_record_fn)(vlog_msg_t * msg);

typedef struct vlog_record_s {
	char name[MAXLEN_PATH + 1];
	vlog_record_fn output;
} vlog_record_t;

vlog_record_t *vlog_record_new(const char *name, vlog_record_fn output);
void vlog_record_del(vlog_record_t *a_record);
void vlog_record_profile(vlog_record_t *a_record, int flag);

#endif
