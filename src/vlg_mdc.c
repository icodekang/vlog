#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "vlg_mdc.h"
#include "vlg_defs.h"

void vlog_mdc_profile(vlog_mdc_t *a_mdc, int flag)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_mdc_kv_t *a_mdc_kv;

	vlg_assert(a_mdc,);
	vlg_profile(flag, "---mdc[%p]---", a_mdc);

	vlg_hashtable_foreach(a_mdc->tab, a_entry) {
		a_mdc_kv = a_entry->value;
		vlg_profile(flag, "----mdc_kv[%p][%s]-[%s]----",
				a_mdc_kv,
				a_mdc_kv->key, a_mdc_kv->value);
	}
	return;
}

void vlog_mdc_del(vlog_mdc_t * a_mdc)
{
	vlg_assert(a_mdc,);
	if (a_mdc->tab) vlg_hashtable_del(a_mdc->tab);
	vlg_debug("vlog_mdc_del[%p]", a_mdc);
    free(a_mdc);
	return;
}

static void vlog_mdc_kv_del(vlog_mdc_kv_t * a_mdc_kv)
{
	vlg_debug("vlog_mdc_kv_del[%p]", a_mdc_kv);
    free(a_mdc_kv);
}

static vlog_mdc_kv_t *vlog_mdc_kv_new(const char *key, const char *value)
{
	vlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = calloc(1, sizeof(vlog_mdc_kv_t));
	if (!a_mdc_kv) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	snprintf(a_mdc_kv->key, sizeof(a_mdc_kv->key), "%s", key);
	a_mdc_kv->value_len = snprintf(a_mdc_kv->value, sizeof(a_mdc_kv->value), "%s", value);
	return a_mdc_kv;
}

vlog_mdc_t *vlog_mdc_new(void)
{
	vlog_mdc_t *a_mdc;

	a_mdc = calloc(1, sizeof(vlog_mdc_t));
	if (!a_mdc) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_mdc->tab = vlg_hashtable_new(20,
			      vlg_hashtable_str_hash,
			      vlg_hashtable_str_equal, NULL,
			      (vlg_hashtable_del_fn) vlog_mdc_kv_del);
	if (!a_mdc->tab) {
		vlg_error("vlg_hashtable_new fail");
		goto err;
	}

	//vlog_mdc_profile(a_mdc, vlg_DEBUG);
	return a_mdc;
err:
	vlog_mdc_del(a_mdc);
	return NULL;
}

int vlog_mdc_put(vlog_mdc_t * a_mdc, const char *key, const char *value)
{
	vlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = vlog_mdc_kv_new(key, value);
	if (!a_mdc_kv) {
		vlg_error("vlog_mdc_kv_new failed");
		return -1;
	}

	if (vlg_hashtable_put(a_mdc->tab, a_mdc_kv->key, a_mdc_kv)) {
		vlg_error("vlg_hashtable_put fail");
		vlog_mdc_kv_del(a_mdc_kv);
		return -1;
	}

	return 0;
}

void vlog_mdc_clean(vlog_mdc_t * a_mdc)
{
	vlg_hashtable_clean(a_mdc->tab);
	return;
}

char *vlog_mdc_get(vlog_mdc_t * a_mdc, const char *key)
{
	vlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = vlg_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		vlg_error("vlg_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv->value;
	}
}

vlog_mdc_kv_t *vlog_mdc_get_kv(vlog_mdc_t * a_mdc, const char *key)
{
	vlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = vlg_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		vlg_error("vlg_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv;
	}
}

void vlog_mdc_remove(vlog_mdc_t * a_mdc, const char *key)
{
	vlg_hashtable_remove(a_mdc->tab, key);
	return;
}
