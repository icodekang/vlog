#ifndef __vlog_thread_h
#define  __vlog_thread_h

#include "vg_defs.h"
#include "vg_event.h"
#include "vg_buf.h"
#include "vg_mdc.h"

typedef struct {
	int init_version;
	vlog_mdc_t *mdc;
	vlog_event_t *event;

	vlog_buf_t *pre_path_buf;
	vlog_buf_t *path_buf;
	vlog_buf_t *archive_path_buf;
	vlog_buf_t *pre_msg_buf;
	vlog_buf_t *msg_buf;
} vlog_thread_t;


void vlog_thread_del(vlog_thread_t * a_thread);
void vlog_thread_profile(vlog_thread_t * a_thread, int flag);
vlog_thread_t *vlog_thread_new(int init_version,
			size_t buf_size_min, size_t buf_size_max, int time_cache_count);

int vlog_thread_rebuild_msg_buf(vlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max);
int vlog_thread_rebuild_event(vlog_thread_t * a_thread, int time_cache_count);

#endif
