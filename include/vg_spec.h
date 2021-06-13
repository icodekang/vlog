#ifndef __vlog_spec_h
#define __vlog_spec_h

#include "vg_event.h"
#include "vg_buf.h"
#include "vg_thread.h"

typedef struct vlog_spec_s vlog_spec_t;

/* write buf, according to each spec's Conversion Characters */
typedef int (*vlog_spec_write_fn) (vlog_spec_t * a_spec,
			 	vlog_thread_t * a_thread,
			 	vlog_buf_t * a_buf);

/* gen a_thread->msg or gen a_thread->path by using write_fn */
typedef int (*vlog_spec_gen_fn) (vlog_spec_t * a_spec,
				vlog_thread_t * a_thread);

struct vlog_spec_s {
	char *str;
	int len;

	char time_fmt[MAXLEN_CFG_LINE + 1];
	int time_cache_index;
	char mdc_key[MAXLEN_PATH + 1];

	char print_fmt[MAXLEN_CFG_LINE + 1];
	int left_adjust;
	int left_fill_zeros;
	size_t max_width;
	size_t min_width;

	vlog_spec_write_fn write_buf;
	vlog_spec_gen_fn gen_msg;
	vlog_spec_gen_fn gen_path;
	vlog_spec_gen_fn gen_archive_path;
};

vlog_spec_t *vlog_spec_new(char *pattern_start, char **pattern_end, int * time_cache_count);
void vlog_spec_del(vlog_spec_t * a_spec);
void vlog_spec_profile(vlog_spec_t * a_spec, int flag);

#define vlog_spec_gen_msg(a_spec, a_thread) \
	a_spec->gen_msg(a_spec, a_thread)

#define vlog_spec_gen_path(a_spec, a_thread) \
	a_spec->gen_path(a_spec, a_thread)

#define vlog_spec_gen_archive_path(a_spec, a_thread) \
	a_spec->gen_archive_path(a_spec, a_thread)

#endif
