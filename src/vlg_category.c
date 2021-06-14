#include "vlg_fmacros.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vlg_category.h"
#include "vlg_rule.h"
#include "vlg_defs.h"

void vlog_category_profile(vlog_category_t *a_category, int flag)
{
	int i;
	vlog_rule_t *a_rule;

	vlg_assert(a_category,);
	vlg_profile(flag, "--category[%p][%s][%p]--",
			a_category,
			a_category->name,
			a_category->fit_rules);
	if (a_category->fit_rules) {
		vlg_arraylist_foreach(a_category->fit_rules, i, a_rule) {
			vlog_rule_profile(a_rule, flag);
		}
	}
	return;
}

void vlog_category_del(vlog_category_t * a_category)
{
	vlg_assert(a_category,);
	if (a_category->fit_rules) vlg_arraylist_del(a_category->fit_rules);
	vlg_debug("vlog_category_del[%p]", a_category);
    free(a_category);
	return;
}

/* overlap one rule's level bitmap to cateogry,
 * so category can judge whether a log level will be output by itself
 * It is safe when configure is reloaded, when rule will be released an recreated
 */
static void vlog_cateogry_overlap_bitmap(vlog_category_t * a_category, vlog_rule_t *a_rule)
{
	int i;
	for(i = 0; i < sizeof(a_rule->level_bitmap); i++) {
		a_category->level_bitmap[i] |= a_rule->level_bitmap[i];
	}
}

static int vlog_category_obtain_rules(vlog_category_t * a_category, vlg_arraylist_t * rules)
{
	int i;
	int count = 0;
	int fit = 0;
	vlog_rule_t *a_rule;
	vlog_rule_t *wastebin_rule = NULL;

	/* before set, clean last fit rules first */
	if (a_category->fit_rules) vlg_arraylist_del(a_category->fit_rules);

	memset(a_category->level_bitmap, 0x00, sizeof(a_category->level_bitmap));

	a_category->fit_rules = vlg_arraylist_new(NULL);
	if (!(a_category->fit_rules)) {
		vlg_error("vlg_arraylist_new fail");
		return -1;
	}

	/* get match rules from all rules */
	vlg_arraylist_foreach(rules, i, a_rule) {
		fit = vlog_rule_match_category(a_rule, a_category->name);
		if (fit) {
			if (vlg_arraylist_add(a_category->fit_rules, a_rule)) {
				vlg_error("vlg_arrylist_add fail");
				goto err;
			}
			vlog_cateogry_overlap_bitmap(a_category, a_rule);
			count++;
		}

		if (vlog_rule_is_wastebin(a_rule)) {
			wastebin_rule = a_rule;
		}
	}

	if (count == 0) {
		if (wastebin_rule) {
			vlg_debug("category[%s], no match rules, use wastebin_rule", a_category->name);
			if (vlg_arraylist_add(a_category->fit_rules, wastebin_rule)) {
				vlg_error("vlg_arrylist_add fail");
				goto err;
			}
			vlog_cateogry_overlap_bitmap(a_category, wastebin_rule);
			count++;
		} else {
			vlg_debug("category[%s], no match rules & no wastebin_rule", a_category->name);
		}
	}

	return 0;
err:
	vlg_arraylist_del(a_category->fit_rules);
	a_category->fit_rules = NULL;
	return -1;
}

vlog_category_t *vlog_category_new(const char *name, vlg_arraylist_t * rules)
{
	size_t len;
	vlog_category_t *a_category;

	vlg_assert(name, NULL);
	vlg_assert(rules, NULL);

	len = strlen(name);
	if (len > sizeof(a_category->name) - 1) {
		vlg_error("name[%s] too long", name);
		return NULL;
	}
	a_category = calloc(1, sizeof(vlog_category_t));
	if (!a_category) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}
	strcpy(a_category->name, name);
	a_category->name_len = len;
	if (vlog_category_obtain_rules(a_category, rules)) {
		vlg_error("vlog_category_fit_rules fail");
		goto err;
	}

	vlog_category_profile(a_category, vlg_DEBUG);
	return a_category;
err:
	vlog_category_del(a_category);
	return NULL;
}

/* update success: fit_rules 1, fit_rules_backup 1 */
/* update fail: fit_rules 0, fit_rules_backup 1 */
int vlog_category_update_rules(vlog_category_t * a_category, vlg_arraylist_t * new_rules)
{
	vlg_assert(a_category, -1);
	vlg_assert(new_rules, -1);

	/* 1st, mv fit_rules fit_rules_backup */
	if (a_category->fit_rules_backup) vlg_arraylist_del(a_category->fit_rules_backup);
	a_category->fit_rules_backup = a_category->fit_rules;
	a_category->fit_rules = NULL;

	memcpy(a_category->level_bitmap_backup, a_category->level_bitmap,
			sizeof(a_category->level_bitmap));
	
	/* 2nd, obtain new_rules to fit_rules */
	if (vlog_category_obtain_rules(a_category, new_rules)) {
		vlg_error("vlog_category_obtain_rules fail");
		a_category->fit_rules = NULL;
		return -1;
	}

	/* keep the fit_rules_backup not change, return */
	return 0;
}

/* commit fail: fit_rules_backup != 0 */
/* commit success: fit_rules 1, fit_rules_backup 0 */
void vlog_category_commit_rules(vlog_category_t * a_category)
{
	vlg_assert(a_category,);
	if (!a_category->fit_rules_backup) {
		vlg_warn("a_category->fit_rules_backup is NULL, never update before");
		return;
	}

	vlg_arraylist_del(a_category->fit_rules_backup);
	a_category->fit_rules_backup = NULL;
	memset(a_category->level_bitmap_backup, 0x00,
			sizeof(a_category->level_bitmap_backup));
	return;
}

/* rollback fail: fit_rules_backup != 0 */
/* rollback success: fit_rules 1, fit_rules_backup 0 */
/* so whether update succes or not, make things back to old */
void vlog_category_rollback_rules(vlog_category_t * a_category)
{
	vlg_assert(a_category,);
	if (!a_category->fit_rules_backup) {
		vlg_warn("a_category->fit_rules_backup in NULL, never update before");
		return;
	}

	if (a_category->fit_rules) {
		/* update success, rm new and backup */
		vlg_arraylist_del(a_category->fit_rules);
		a_category->fit_rules = a_category->fit_rules_backup;
		a_category->fit_rules_backup = NULL;
	} else {
		/* update fail, just backup */
		a_category->fit_rules = a_category->fit_rules_backup;
		a_category->fit_rules_backup = NULL;
	}

	memcpy(a_category->level_bitmap, a_category->level_bitmap_backup,
			sizeof(a_category->level_bitmap));
	memset(a_category->level_bitmap_backup, 0x00,
			sizeof(a_category->level_bitmap_backup));
	
	return; /* always success */
}

int vlog_category_output(vlog_category_t * a_category, vlog_thread_t * a_thread)
{
	int i;
	int rc = 0;
	vlog_rule_t *a_rule;

	/* go through all match rules to output */
	vlg_arraylist_foreach(a_category->fit_rules, i, a_rule) {
		rc = vlog_rule_output(a_rule, a_thread);
	}

	return rc;
}
