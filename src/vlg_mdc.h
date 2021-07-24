#ifndef __vlog_mdc_h
#define __vlog_mdc_h

#include "vlg_defs.h"

typedef struct vlog_mdc_s vlog_mdc_t;
struct vlog_mdc_s {
	vlg_hashtable_t *tab;
};

vlog_mdc_t *vlog_mdc_new(void);
void vlog_mdc_del(vlog_mdc_t * a_mdc);
void vlog_mdc_profile(vlog_mdc_t *a_mdc, int flag);

void vlog_mdc_clean(vlog_mdc_t * a_mdc);
int vlog_mdc_put(vlog_mdc_t * a_mdc, const char *key, const char *value);
char *vlog_mdc_get(vlog_mdc_t * a_mdc, const char *key);
void vlog_mdc_remove(vlog_mdc_t * a_mdc, const char *key);

typedef struct vlog_mdc_kv_s {
	char key[MAXLEN_PATH + 1];
	char value[MAXLEN_PATH + 1];
	size_t value_len;
} vlog_mdc_kv_t;

vlog_mdc_kv_t *vlog_mdc_get_kv(vlog_mdc_t * a_mdc, const char *key);

#endif
