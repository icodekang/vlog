#ifndef __vlog_category_table_h
#define __vlog_category_table_h

#include "vlg_defs.h"
#include "vlg_category.h"

vlg_hashtable_t *vlog_category_table_new(void);
void vlog_category_table_del(vlg_hashtable_t * categories);
void vlog_category_table_profile(vlg_hashtable_t * categories, int flag);

/* if none, create new and return */
vlog_category_t *vlog_category_table_fetch_category(
			vlg_hashtable_t * categories,
		 	const char *category_name, vlg_arraylist_t * rules);

int vlog_category_table_update_rules(vlg_hashtable_t * categories, vlg_arraylist_t * new_rules);
void vlog_category_table_commit_rules(vlg_hashtable_t * categories);
void vlog_category_table_rollback_rules(vlg_hashtable_t * categories);

#endif
