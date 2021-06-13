#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "syslog.h"

#include "vg_defs.h"
#include "vg_level.h"
#include "vg_level_list.h"

/* vlog_level_list == vg_arraylist_t<vlog_level_t> */

void vlog_level_list_profile(vg_arraylist_t *levels, int flag)
{
	int i;
	vlog_level_t *a_level;

	vg_assert(levels,);
	vg_profile(flag, "--level_list[%p]--", levels);
	vg_arraylist_foreach(levels, i, a_level) {
		/* skip empty slots */
		if (a_level) vlog_level_profile(a_level, flag);
	}
	return;
}

void vlog_level_list_del(vg_arraylist_t *levels)
{
	vg_assert(levels,);
	vg_arraylist_del(levels);
	vg_debug("vg_level_list_del[%p]", levels);
	return;
}

static int vlog_level_list_set_default(vg_arraylist_t *levels)
{
	return vlog_level_list_set(levels, "* = 0, LOG_INFO")
	|| vlog_level_list_set(levels, "DEBUG = 20, LOG_DEBUG")
	|| vlog_level_list_set(levels, "INFO = 40, LOG_INFO")
	|| vlog_level_list_set(levels, "NOTICE = 60, LOG_NOTICE")
	|| vlog_level_list_set(levels, "WARN = 80, LOG_WARNING")
	|| vlog_level_list_set(levels, "ERROR = 100, LOG_ERR")
	|| vlog_level_list_set(levels, "FATAL = 120, LOG_ALERT")
	|| vlog_level_list_set(levels, "UNKNOWN = 254, LOG_ERR")
	|| vlog_level_list_set(levels, "! = 255, LOG_INFO");
}

vg_arraylist_t *vlog_level_list_new(void)
{
	vg_arraylist_t *levels;

	levels = vg_arraylist_new((vg_arraylist_del_fn)vlog_level_del);
	if (!levels) {
		vg_error("vg_arraylist_new fail");
		return NULL;
	}

	if (vlog_level_list_set_default(levels)) {
		vg_error("vlog_level_set_default fail");
		goto err;
	}

	//vlog_level_list_profile(levels, vg_DEBUG);
	return levels;
err:
	vg_arraylist_del(levels);
	return NULL;
}

int vlog_level_list_set(vg_arraylist_t *levels, char *line)
{
	vlog_level_t *a_level;

	a_level = vlog_level_new(line);
	if (!a_level) {
		vg_error("vlog_level_new fail");
		return -1;
	}

	if (vg_arraylist_set(levels, a_level->int_level, a_level)) {
		vg_error("vg_arraylist_set fail");
		goto err;
	}

	return 0;
err:
	vg_error("line[%s]", line);
	vlog_level_del(a_level);
	return -1;
}

vlog_level_t *vlog_level_list_get(vg_arraylist_t *levels, int l)
{
	vlog_level_t *a_level;

#if 0
	if ((l <= 0) || (l > 254)) {
		/* illegal input from vlog() */
		vg_error("l[%d] not in (0,254), set to UNKOWN", l);
		l = 254;
	}
#endif

	a_level = vg_arraylist_get(levels, l);
	if (a_level) {
		return a_level;
	} else {
		/* empty slot */
		vg_error("l[%d] not in (0,254), or has no level defined,"
			"see configure file define, set to UNKOWN", l);
		return vg_arraylist_get(levels, 254);
	}
}

int vlog_level_list_atoi(vg_arraylist_t *levels, char *str)
{
	int i;
	vlog_level_t *a_level;

	if (str == NULL || *str == '\0') {
		vg_error("str is [%s], can't find level", str);
		return -1;
	}

	vg_arraylist_foreach(levels, i, a_level) {
		if (a_level && STRICMP(str, ==, a_level->str_uppercase)) {
			return i;
		}
	}

	vg_error("str[%s] can't found in level list", str);
	return -1;
}

