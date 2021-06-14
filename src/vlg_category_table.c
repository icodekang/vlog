#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vlg_defs.h"
#include "vlg_category_table.h"

void vlog_category_table_profile(vlg_hashtable_t * categories, int flag)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vlg_assert(categories,);
	vlg_profile(flag, "-category_table[%p]-", categories);
	vlg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_profile(a_category, flag);
	}
	return;
}

void vlog_category_table_del(vlg_hashtable_t * categories)
{
	vlg_assert(categories,);
	vlg_hashtable_del(categories);
	vlg_debug("vlog_category_table_del[%p]", categories);
	return;
}

vlg_hashtable_t *vlog_category_table_new(void)
{
	vlg_hashtable_t *categories;

	categories = vlg_hashtable_new(20,
			 (vlg_hashtable_hash_fn) vlg_hashtable_str_hash,
			 (vlg_hashtable_equal_fn) vlg_hashtable_str_equal,
			 NULL, (vlg_hashtable_del_fn) vlog_category_del);
	if (!categories) {
		vlg_error("vlg_hashtable_new fail");
		return NULL;
	} else {
		vlog_category_table_profile(categories, vlg_DEBUG);
		return categories;
	}
}

int vlog_category_table_update_rules(vlg_hashtable_t * categories, vlg_arraylist_t * new_rules)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vlg_assert(categories, -1);
	vlg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		if (vlog_category_update_rules(a_category, new_rules)) {
			vlg_error("vlog_category_update_rules fail, try rollback");
			return -1;
		}
	}
	return 0;
}

void vlog_category_table_commit_rules(vlg_hashtable_t * categories)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vlg_assert(categories,);
	vlg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_commit_rules(a_category);
	}
	return;
}

void vlog_category_table_rollback_rules(vlg_hashtable_t * categories)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vlg_assert(categories,);
	vlg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_rollback_rules(a_category);
	}
	return;
}

vlog_category_t *vlog_category_table_fetch_category(vlg_hashtable_t * categories,
			const char *category_name, vlg_arraylist_t * rules)
{
	vlog_category_t *a_category;

	vlg_assert(categories, NULL);

	/* 1st find category in global category map */
	a_category = vlg_hashtable_get(categories, category_name);
	if (a_category) return a_category;

	/* else not found, create one */
	a_category = vlog_category_new(category_name, rules);
	if (!a_category) {
		vlg_error("vlg_category_new fail");
		return NULL;
	}

	if(vlg_hashtable_put(categories, a_category->name, a_category)) {
		vlg_error("vlg_hashtable_put fail");
		goto err;
	}

	return a_category;
err:
	vlog_category_del(a_category);
	return NULL;
}