#ifndef __vlog_category_h
#define __vlog_category_h

#include "vg_defs.h"
#include "vg_thread.h"

typedef struct vlog_category_s {
	char name[MAXLEN_PATH + 1];
	size_t name_len;
	unsigned char level_bitmap[32];
	unsigned char level_bitmap_backup[32];
	vg_arraylist_t *fit_rules;
	vg_arraylist_t *fit_rules_backup;
} vlog_category_t;

vlog_category_t *vlog_category_new(const char *name, vg_arraylist_t * rules);
void vlog_category_del(vlog_category_t * a_category);
void vlog_category_profile(vlog_category_t *a_category, int flag);

int vlog_category_update_rules(vlog_category_t * a_category, vg_arraylist_t * new_rules);
void vlog_category_commit_rules(vlog_category_t * a_category);
void vlog_category_rollback_rules(vlog_category_t * a_category);

int vlog_category_output(vlog_category_t * a_category, vlog_thread_t * a_thread);

#define vlog_category_needless_level(a_category, lv) \
        a_category && !((a_category->level_bitmap[lv/8] >> (7 - lv % 8)) & 0x01)


#endif
