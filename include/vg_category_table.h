#ifndef __vlog_category_table_h
#define __vlog_category_table_h

#include "vg_defs.h"
#include "vg_category.h"

vg_hashtable_t *vlog_category_table_new(void);
void vlog_category_table_del(vg_hashtable_t * categories);
void vlog_category_table_profile(vg_hashtable_t * categories, int flag);

/* if none, create new and return */
vlog_category_t *vlog_category_table_fetch_category(
			vg_hashtable_t * categories,
		 	const char *category_name, vg_arraylist_t * rules);

int vlog_category_table_update_rules(vg_hashtable_t * categories, vg_arraylist_t * new_rules);
void vlog_category_table_commit_rules(vg_hashtable_t * categories);
void vlog_category_table_rollback_rules(vg_hashtable_t * categories);

#endif
