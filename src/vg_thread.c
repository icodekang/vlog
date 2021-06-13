#include <pthread.h>
#include <errno.h>

#include "vg_defs.h"
#include "vg_event.h"
#include "vg_buf.h"
#include "vg_thread.h"
#include "vg_mdc.h"

void vlog_thread_profile(vlog_thread_t * a_thread, int flag)
{
	vg_assert(a_thread,);
	vg_profile(flag, "--thread[%p][%p][%p][%p,%p,%p,%p,%p]--",
			a_thread,
			a_thread->mdc,
			a_thread->event,
			a_thread->pre_path_buf,
			a_thread->path_buf,
			a_thread->archive_path_buf,
			a_thread->pre_msg_buf,
			a_thread->msg_buf);

	vlog_mdc_profile(a_thread->mdc, flag);
	vlog_event_profile(a_thread->event, flag);
	vlog_buf_profile(a_thread->pre_path_buf, flag);
	vlog_buf_profile(a_thread->path_buf, flag);
	vlog_buf_profile(a_thread->archive_path_buf, flag);
	vlog_buf_profile(a_thread->pre_msg_buf, flag);
	vlog_buf_profile(a_thread->msg_buf, flag);
	return;
}

void vlog_thread_del(vlog_thread_t * a_thread)
{
	vg_assert(a_thread,);
	if (a_thread->mdc)
		vlog_mdc_del(a_thread->mdc);
	if (a_thread->event)
		vlog_event_del(a_thread->event);
	if (a_thread->pre_path_buf)
		vlog_buf_del(a_thread->pre_path_buf);
	if (a_thread->path_buf)
		vlog_buf_del(a_thread->path_buf);
	if (a_thread->archive_path_buf)
		vlog_buf_del(a_thread->archive_path_buf);
	if (a_thread->pre_msg_buf)
		vlog_buf_del(a_thread->pre_msg_buf);
	if (a_thread->msg_buf)
		vlog_buf_del(a_thread->msg_buf);

	vg_debug("vlog_thread_del[%p]", a_thread);
    free(a_thread);
	return;
}

vlog_thread_t *vlog_thread_new(int init_version, size_t buf_size_min, size_t buf_size_max, int time_cache_count)
{
	vlog_thread_t *a_thread;

	a_thread = calloc(1, sizeof(vlog_thread_t));
	if (!a_thread) {
		vg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_thread->init_version = init_version;

	a_thread->mdc = vlog_mdc_new();
	if (!a_thread->mdc) {
		vg_error("vlog_mdc_new fail");
		goto err;
	}

	a_thread->event = vlog_event_new(time_cache_count);
	if (!a_thread->event) {
		vg_error("vlog_event_new fail");
		goto err;
	}

	a_thread->pre_path_buf = vlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->pre_path_buf) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	a_thread->path_buf = vlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->path_buf) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	a_thread->archive_path_buf = vlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->archive_path_buf) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	a_thread->pre_msg_buf = vlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->pre_msg_buf) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	a_thread->msg_buf = vlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->msg_buf) {
		vg_error("vlog_buf_new fail");
		goto err;
	}


	//vlog_thread_profile(a_thread, vg_DEBUG);
	return a_thread;
err:
	vlog_thread_del(a_thread);
	return NULL;
}

int vlog_thread_rebuild_msg_buf(vlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max)
{
	vlog_buf_t *pre_msg_buf_new = NULL;
	vlog_buf_t *msg_buf_new = NULL;
	vg_assert(a_thread, -1);

	if ( (a_thread->msg_buf->size_min == buf_size_min)
		&& (a_thread->msg_buf->size_max == buf_size_max)) {
		vg_debug("buf size not changed, no need rebuild");
		return 0;
	}

	pre_msg_buf_new = vlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!pre_msg_buf_new) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	msg_buf_new = vlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!msg_buf_new) {
		vg_error("vlog_buf_new fail");
		goto err;
	}

	vlog_buf_del(a_thread->pre_msg_buf);
	a_thread->pre_msg_buf = pre_msg_buf_new;

	vlog_buf_del(a_thread->msg_buf);
	a_thread->msg_buf = msg_buf_new;

	return 0;
err:
	if (pre_msg_buf_new) vlog_buf_del(pre_msg_buf_new);
	if (msg_buf_new) vlog_buf_del(msg_buf_new);
	return -1;
}

int vlog_thread_rebuild_event(vlog_thread_t * a_thread, int time_cache_count)
{
	vlog_event_t *event_new = NULL;
	vg_assert(a_thread, -1);

	event_new = vlog_event_new(time_cache_count);
	if (!event_new) {
		vg_error("vlog_event_new fail");
		goto err;
	}

	vlog_event_del(a_thread->event);
	a_thread->event = event_new;
	return 0;
err:
	if (event_new) vlog_event_del(event_new);
	return -1;
}