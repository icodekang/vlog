#ifndef __vlog_rotater_h
#define __vlog_rotater_h

#include "vlg_defs.h"

typedef struct vlog_rotater_s {
	pthread_mutex_t lock_mutex;
	char *lock_file;
	int lock_fd;

	/* single-use members */
	char *base_path;			/* aa.log */
	char *archive_path;			/* aa.#5i.log */
	char glob_path[MAXLEN_PATH + 1];	/* aa.*.log */
	size_t num_start_len;			/* 3, offset to glob_path */
	size_t num_end_len;			/* 6, offset to glob_path */
	int num_width;				/* 5 */
	int mv_type;				/* ROLLING or SEQUENCE */
	int max_count;
	vlg_arraylist_t *files;
} vlog_rotater_t;

vlog_rotater_t *vlog_rotater_new(char *lock_file);
void vlog_rotater_del(vlog_rotater_t *a_rotater);

/*
 * return
 * -1	fail
 * 0	no rotate, or rotate and success
 */
int vlog_rotater_rotate(vlog_rotater_t *a_rotater,
		char *base_path, size_t msg_len,
		char *archive_path, long archive_max_size, int archive_max_count);

void vlog_rotater_profile(vlog_rotater_t *a_rotater, int flag);

#endif
