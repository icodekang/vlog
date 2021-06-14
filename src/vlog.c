#include "vlg_fmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

#include "vlg_conf.h"
#include "vlg_category_table.h"
#include "vlg_record_table.h"
#include "vlg_mdc.h"
#include "vlg_defs.h"
#include "vlg_rule.h"
#include "vlg_version.h"

extern char *vlog_git_sha1;

static pthread_rwlock_t vlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_key_t vlog_thread_key;
static vlg_hashtable_t *vlog_env_categories;
static vlg_hashtable_t *vlog_env_records;
static vlog_category_t *vlog_default_category;
static size_t vlog_env_reload_conf_count;
static int vlog_env_is_init = 0;
static int vlog_env_init_version = 0;

vlog_conf_t *vlog_env_conf;

/* inner no need thread-safe */
static void vlog_fini_inner(void)
{
	/* pthread_key_delete(vlog_thread_key); */
	/* never use pthread_key_delete,
	 * it will cause other thread can't release vlog_thread_t 
	 * after one thread call pthread_key_delete
	 * also key not init will cause a core dump
	 */
	
	if (vlog_env_categories) vlog_category_table_del(vlog_env_categories);
	vlog_env_categories = NULL;
	vlog_default_category = NULL;
	if (vlog_env_records) vlog_record_table_del(vlog_env_records);
	vlog_env_records = NULL;
	if (vlog_env_conf) vlog_conf_del(vlog_env_conf);
	vlog_env_conf = NULL;
	return;
}

static void vlog_clean_rest_thread(void)
{
	vlog_thread_t *a_thread;
	a_thread = pthread_getspecific(vlog_thread_key);
	if (!a_thread) return;
	vlog_thread_del(a_thread);
	return;
}

static int vlog_init_inner(const char *config)
{
	int rc = 0;

	/* the 1st time in the whole process do init */
	if (vlog_env_init_version == 0) {
		/* clean up is done by OS when a thread call pthread_exit */
		rc = pthread_key_create(&vlog_thread_key, (void (*) (void *)) vlog_thread_del);
		if (rc) {
			vlg_error("pthread_key_create fail, rc[%d]", rc);
			goto err;
		}

		/* if some thread do not call pthread_exit, like main thread
		 * atexit will clean it 
		 */
		rc = atexit(vlog_clean_rest_thread);
		if (rc) {
			vlg_error("atexit fail, rc[%d]", rc);
			goto err;
		}
		vlog_env_init_version++;
	} /* else maybe after vlog_fini() and need not create pthread_key */

	vlog_env_conf = vlog_conf_new(config);
	if (!vlog_env_conf) {
		vlg_error("vlog_conf_new[%s] fail", config);
		goto err;
	}

	vlog_env_categories = vlog_category_table_new();
	if (!vlog_env_categories) {
		vlg_error("vlog_category_table_new fail");
		goto err;
	}

	vlog_env_records = vlog_record_table_new();
	if (!vlog_env_records) {
		vlg_error("vlog_record_table_new fail");
		goto err;
	}

	return 0;
err:
	vlog_fini_inner();
	return -1;
}

int vlog_init(const char *config)
{
	int rc;
	vlg_debug("------vlog_init start------");
	vlg_debug("------compile time[%s %s], version[%s]------", __DATE__, __TIME__, vlog_VERSION);

	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (vlog_env_is_init) {
		vlg_error("already init, use vlog_reload pls");
		goto err;
	}


	if (vlog_init_inner(config)) {
		vlg_error("vlog_init_inner[%s] fail", config);
		goto err;
	}

	vlog_env_is_init = 1;
	vlog_env_init_version++;

	vlg_debug("------vlog_init success end------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	vlg_error("------vlog_init fail end------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

int dvlog_init(const char *config, const char *cname)
{
	int rc = 0;
	vlg_debug("------dvlog_init start------");
	vlg_debug("------compile time[%s %s], version[%s]------",
			__DATE__, __TIME__, vlog_VERSION);

	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (vlog_env_is_init) {
		vlg_error("already init, use vlog_reload pls");
		goto err;
	}

	if (vlog_init_inner(config)) {
		vlg_error("vlog_init_inner[%s] fail", config);
		goto err;
	}

	vlog_default_category = vlog_category_table_fetch_category(
				vlog_env_categories,
				cname,
				vlog_env_conf->rules);
	if (!vlog_default_category) {
		vlg_error("vlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	vlog_env_is_init = 1;
	vlog_env_init_version++;

	vlg_debug("------dvlog_init success end------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	vlg_error("------dvlog_init fail end------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

int vlog_reload(const char *config)
{
	int rc = 0;
	int i = 0;
	vlog_conf_t *new_conf = NULL;
	vlog_rule_t *a_rule;
	int c_up = 0;

	vlg_debug("------vlog_reload start------");
	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto quit;
	}

	/* use last conf file */
	if (config == NULL) config = vlog_env_conf->file;

	/* reach reload period */
	if (config == (char*)-1) {
		/* test again, avoid other threads already reloaded */
		if (vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period) {
			config = vlog_env_conf->file;
		} else {
			/* do nothing, already done */
			goto quit;
		}
	}

	/* reset counter, whether automaticlly or mannually */
	vlog_env_reload_conf_count = 0;

	new_conf = vlog_conf_new(config);
	if (!new_conf) {
		vlg_error("vlog_conf_new fail");
		goto err;
	}

	vlg_arraylist_foreach(new_conf->rules, i, a_rule) {
		vlog_rule_set_record(a_rule, vlog_env_records);
	}

	if (vlog_category_table_update_rules(vlog_env_categories, new_conf->rules)) {
		c_up = 0;
		vlg_error("vlog_category_table_update fail");
		goto err;
	} else {
		c_up = 1;
	}

	vlog_env_init_version++;

	if (c_up) vlog_category_table_commit_rules(vlog_env_categories);
	vlog_conf_del(vlog_env_conf);
	vlog_env_conf = new_conf;
	vlg_debug("------vlog_reload success, total init verison[%d] ------", vlog_env_init_version);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	/* fail, roll back everything */
	vlg_warn("vlog_reload fail, use old conf file, still working");
	if (new_conf) vlog_conf_del(new_conf);
	if (c_up) vlog_category_table_rollback_rules(vlog_env_categories);
	vlg_error("------vlog_reload fail, total init version[%d] ------", vlog_env_init_version);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
quit:
	vlg_debug("------vlog_reload do nothing------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
}

void vlog_fini(void)
{
	int rc = 0;

	vlg_debug("------vlog_fini start------");
	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}

	if (!vlog_env_is_init) {
		vlg_error("before finish, must vlog_init() or dvlog_init() first");
		goto exit;
	}

	vlog_fini_inner();
	vlog_env_is_init = 0;

exit:
	vlg_debug("------vlog_fini end------");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

vlog_category_t *vlog_get_category(const char *cname)
{
	int rc = 0;
	vlog_category_t *a_category = NULL;

	vlg_assert(cname, NULL);
	vlg_debug("------vlog_get_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return NULL;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		a_category = NULL;
		goto err;
	}

	a_category = vlog_category_table_fetch_category(
				vlog_env_categories,
				cname,
				vlog_env_conf->rules);
	if (!a_category) {
		vlg_error("vlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	vlg_debug("------vlog_get_category[%s] success, end------ ", cname);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return a_category;
err:
	vlg_error("------vlog_get_category[%s] fail, end------ ", cname);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return NULL;
}

int dvlog_set_category(const char *cname)
{
	int rc = 0;
	vlg_assert(cname, -1);

	vlg_debug("------dvlog_set_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto err;
	}

	vlog_default_category = vlog_category_table_fetch_category(
				vlog_env_categories,
				cname,
				vlog_env_conf->rules);
	if (!vlog_default_category) {
		vlg_error("vlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	vlg_debug("------dvlog_set_category[%s] end, success------ ", cname);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	vlg_error("------dvlog_set_category[%s] end, fail------ ", cname);
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

#define vlog_fetch_thread(a_thread, fail_goto) do {  \
	int rd = 0;  \
	a_thread = pthread_getspecific(vlog_thread_key);  \
	if (!a_thread) {  \
		a_thread = vlog_thread_new(vlog_env_init_version,  \
				vlog_env_conf->buf_size_min, vlog_env_conf->buf_size_max, \
				vlog_env_conf->time_cache_count); \
		if (!a_thread) {  \
			vlg_error("vlog_thread_new fail");  \
			goto fail_goto;  \
		}  \
  \
		rd = pthread_setspecific(vlog_thread_key, a_thread);  \
		if (rd) {  \
			vlog_thread_del(a_thread);  \
			vlg_error("pthread_setspecific fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
	}  \
  \
	if (a_thread->init_version != vlog_env_init_version) {  \
		/* as mdc is still here, so can not easily del and new */ \
		rd = vlog_thread_rebuild_msg_buf(a_thread, \
				vlog_env_conf->buf_size_min, \
				vlog_env_conf->buf_size_max);  \
		if (rd) {  \
			vlg_error("vlog_thread_resize_msg_buf fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
  \
		rd = vlog_thread_rebuild_event(a_thread, vlog_env_conf->time_cache_count);  \
		if (rd) {  \
			vlg_error("vlog_thread_resize_msg_buf fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
		a_thread->init_version = vlog_env_init_version;  \
	}  \
} while (0)

int vlog_put_mdc(const char *key, const char *value)
{
	int rc = 0;
	vlog_thread_t *a_thread;

	vlg_assert(key, -1);
	vlg_assert(value, -1);

	rc = pthread_rwlock_rdlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto err;
	}

	vlog_fetch_thread(a_thread, err);

	if (vlog_mdc_put(a_thread->mdc, key, value)) {
		vlg_error("vlog_mdc_put fail, key[%s], value[%s]", key, value);
		goto err;
	}

	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

char *vlog_get_mdc(char *key)
{
	int rc = 0;
	char *value = NULL;
	vlog_thread_t *a_thread;

	vlg_assert(key, NULL);

	rc = pthread_rwlock_rdlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return NULL;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto err;
	}

	a_thread = pthread_getspecific(vlog_thread_key);
	if (!a_thread) {
		vlg_error("thread not found, maybe not use vlog_put_mdc before");
		goto err;
	}

	value = vlog_mdc_get(a_thread->mdc, key);
	if (!value) {
		vlg_error("key[%s] not found in mdc", key);
		goto err;
	}

	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return value;
err:
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return NULL;
}

void vlog_remove_mdc(char *key)
{
	int rc = 0;
	vlog_thread_t *a_thread;

	vlg_assert(key, );

	rc = pthread_rwlock_rdlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	a_thread = pthread_getspecific(vlog_thread_key);
	if (!a_thread) {
		vlg_error("thread not found, maybe not use vlog_put_mdc before");
		goto exit;
	}

	vlog_mdc_remove(a_thread->mdc, key);

exit:
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

void vlog_clean_mdc(void)
{
	int rc = 0;
	vlog_thread_t *a_thread;

	rc = pthread_rwlock_rdlock(&vlog_env_lock);
	if (rc) {;
		vlg_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	a_thread = pthread_getspecific(vlog_thread_key);
	if (!a_thread) {
		vlg_error("thread not found, maybe not use vlog_put_mdc before");
		goto exit;
	}

	vlog_mdc_clean(a_thread->mdc);

exit:
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

int vlog_level_switch(vlog_category_t * category, int level)
{
    // This is NOT thread safe.
    memset(category->level_bitmap, 0x00, sizeof(category->level_bitmap));
    category->level_bitmap[level / 8] |= ~(0xFF << (8 - level % 8));
    memset(category->level_bitmap + level / 8 + 1, 0xFF,
	    sizeof(category->level_bitmap) -  level / 8 - 1);

    return 0;
}

void vvlog(vlog_category_t * category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
	vlog_thread_t *a_thread;

	/* The bitmap determination here is not under the protection of rdlock.
	 * It may be changed by other CPU by vlog_reload() halfway.
	 *
	 * Old or strange value may be read here,
	 * but it is safe, the bitmap is valid as long as category exist,
	 * And will be the right value after vlog_reload()
	 *
	 * For speed up, if one log will not be ouput,
	 * There is no need to aquire rdlock.
	 */
	if (vlog_category_needless_level(category, level)) return;

	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	vlog_fetch_thread(a_thread, exit);

	vlog_event_set_fmt(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (vlog_category_output(category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

void hvlog(vlog_category_t *category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	vlog_thread_t *a_thread;

	if (vlog_category_needless_level(category, level)) return;

	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	vlog_fetch_thread(a_thread, exit);

	vlog_event_set_hex(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (vlog_category_output(category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

/* for speed up, copy from vvlog */
void vdvlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
	vlog_thread_t *a_thread;

	if (vlog_category_needless_level(vlog_default_category, level)) return;

	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!vlog_default_category) {
		vlg_error("vlog_default_category is null,"
			"dvlog_init() or dvlog_set_cateogry() is not called above");
		goto exit;
	}

	vlog_fetch_thread(a_thread, exit);

	vlog_event_set_fmt(a_thread->event,
		vlog_default_category->name, vlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (vlog_category_output(vlog_default_category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

void hdvlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	vlog_thread_t *a_thread;

	if (vlog_category_needless_level(vlog_default_category, level)) return;

	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!vlog_default_category) {
		vlg_error("vlog_default_category is null,"
			"dvlog_init() or dvlog_set_cateogry() is not called above");
		goto exit;
	}

	vlog_fetch_thread(a_thread, exit);

	vlog_event_set_hex(a_thread->event,
		vlog_default_category->name, vlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (vlog_category_output(vlog_default_category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

void vlog(vlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, ...)
{
	vlog_thread_t *a_thread;
	va_list args;

	if (category && vlog_category_needless_level(category, level)) return;

	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	vlog_fetch_thread(a_thread, exit);

	va_start(args, format);
	vlog_event_set_fmt(a_thread->event, category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);
	if (vlog_category_output(category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		va_end(args);
		goto exit;
	}
	va_end(args);

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

void dvlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, ...)
{
	vlog_thread_t *a_thread;
	va_list args;


	pthread_rwlock_rdlock(&vlog_env_lock);

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!vlog_default_category) {
		vlg_error("vlog_default_category is null,"
			"dvlog_init() or dvlog_set_cateogry() is not called above");
		goto exit;
	}

	if (vlog_category_needless_level(vlog_default_category, level)) goto exit;

	vlog_fetch_thread(a_thread, exit);

	va_start(args, format);
	vlog_event_set_fmt(a_thread->event,
		vlog_default_category->name, vlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (vlog_category_output(vlog_default_category, a_thread)) {
		vlg_error("vlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		va_end(args);
		goto exit;
	}
	va_end(args);

	if (vlog_env_conf->reload_conf_period &&
		++vlog_env_reload_conf_count > vlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&vlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&vlog_env_lock);
	/* will be wrlock, so after unlock */
	if (vlog_reload((char *)-1)) {
		vlg_error("reach reload-conf-period but vlog_reload fail, vlog-chk-conf [file] see detail");
	}
	return;
}

void vlog_profile(void)
{
	int rc = 0;
	rc = pthread_rwlock_rdlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}
	vlg_warn("------vlog_profile start------ ");
	vlg_warn("is init:[%d]", vlog_env_is_init);
	vlg_warn("init version:[%d]", vlog_env_init_version);
	vlog_conf_profile(vlog_env_conf, vlg_WARN);
	vlog_record_table_profile(vlog_env_records, vlg_WARN);
	vlog_category_table_profile(vlog_env_categories, vlg_WARN);
	if (vlog_default_category) {
		vlg_warn("-default_category-");
		vlog_category_profile(vlog_default_category, vlg_WARN);
	}
	vlg_warn("------vlog_profile end------ ");
	rc = pthread_rwlock_unlock(&vlog_env_lock);
	if (rc) {
		vlg_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

int vlog_set_record(const char *rname, vlog_record_fn record_output)
{
	int rc = 0;
	int rd = 0;
	vlog_rule_t *a_rule;
	vlog_record_t *a_record;
	int i = 0;

	vlg_assert(rname, -1);
	vlg_assert(record_output, -1);

	rd = pthread_rwlock_wrlock(&vlog_env_lock);
	if (rd) {
		vlg_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return -1;
	}

	if (!vlog_env_is_init) {
		vlg_error("never call vlog_init() or dvlog_init() before");
		goto vlog_set_record_exit;
	}

	a_record = vlog_record_new(rname, record_output);
	if (!a_record) {
		rc = -1;
		vlg_error("vlog_record_new fail");
		goto vlog_set_record_exit;
	}

	rc = vlg_hashtable_put(vlog_env_records, a_record->name, a_record);
	if (rc) {
		vlog_record_del(a_record);
		vlg_error("vlg_hashtable_put fail");
		goto vlog_set_record_exit;
	}

	vlg_arraylist_foreach(vlog_env_conf->rules, i, a_rule) {
		vlog_rule_set_record(a_rule, vlog_env_records);
	}

      vlog_set_record_exit:
	rd = pthread_rwlock_unlock(&vlog_env_lock);
	if (rd) {
		vlg_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return rc;
}

int vlog_level_enabled(vlog_category_t *category, const int level)
{
	return category && (vlog_category_needless_level(category, level) == 0);
}

const char *vlog_version(void) { return vlog_VERSION; }
