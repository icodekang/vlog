#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vg_defs.h"
#include "vg_category_table.h"

void vlog_category_table_profile(vg_hashtable_t * categories, int flag)
{
	vg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vg_assert(categories,);
	vg_profile(flag, "-category_table[%p]-", categories);
	vg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_profile(a_category, flag);
	}
	return;
}

void vlog_category_table_del(vg_hashtable_t * categories)
{
	vg_assert(categories,);
	vg_hashtable_del(categories);
	vg_debug("vlog_category_table_del[%p]", categories);
	return;
}

vg_hashtable_t *vlog_category_table_new(void)
{
	vg_hashtable_t *categories;

	categories = vg_hashtable_new(20,
			 (vg_hashtable_hash_fn) vg_hashtable_str_hash,
			 (vg_hashtable_equal_fn) vg_hashtable_str_equal,
			 NULL, (vg_hashtable_del_fn) vlog_category_del);
	if (!categories) {
		vg_error("vg_hashtable_new fail");
		return NULL;
	} else {
		vlog_category_table_profile(categories, vg_DEBUG);
		return categories;
	}
}

int vlog_category_table_update_rules(vg_hashtable_t * categories, vg_arraylist_t * new_rules)
{
	vg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vg_assert(categories, -1);
	vg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		if (vlog_category_update_rules(a_category, new_rules)) {
			vg_error("vlog_category_update_rules fail, try rollback");
			return -1;
		}
	}
	return 0;
}

void vlog_category_table_commit_rules(vg_hashtable_t * categories)
{
	vg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vg_assert(categories,);
	vg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_commit_rules(a_category);
	}
	return;
}

void vlog_category_table_rollback_rules(vg_hashtable_t * categories)
{
	vg_hashtable_entry_t *a_entry;
	vlog_category_t *a_category;

	vg_assert(categories,);
	vg_hashtable_foreach(categories, a_entry) {
		a_category = (vlog_category_t *) a_entry->value;
		vlog_category_rollback_rules(a_category);
	}
	return;
}

vlog_category_t *vlog_category_table_fetch_category(vg_hashtable_t * categories,
			const char *category_name, vg_arraylist_t * rules)
{
	vlog_category_t *a_category;

	vg_assert(categories, NULL);

	/* 1st find category in global category map */
	a_category = vg_hashtable_get(categories, category_name);
	if (a_category) return a_category;

	/* else not found, create one */
	a_category = vlog_category_new(category_name, rules);
	if (!a_category) {
		vg_error("vg_category_new fail");
		return NULL;
	}

	if(vg_hashtable_put(categories, a_category->name, a_category)) {
		vg_error("vg_hashtable_put fail");
		goto err;
	}

	return a_category;
err:
	vlog_category_del(a_category);
	return NULL;
}