#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "syslog.h"

#include "vlg_defs.h"
#include "vlg_level.h"
#include "vlg_level_list.h"

/* vlog_level_list == vlg_arraylist_t<vlog_level_t> */

void vlog_level_list_profile(vlg_arraylist_t *levels, int flag)
{
	int i;
	vlog_level_t *a_level;

	vlg_assert(levels,);
	vlg_profile(flag, "--level_list[%p]--", levels);
	vlg_arraylist_foreach(levels, i, a_level) {
		/* skip empty slots */
		if (a_level) vlog_level_profile(a_level, flag);
	}
	return;
}

void vlog_level_list_del(vlg_arraylist_t *levels)
{
	vlg_assert(levels,);
	vlg_arraylist_del(levels);
	vlg_debug("vlg_level_list_del[%p]", levels);
	return;
}

static int vlog_level_list_set_default(vlg_arraylist_t *levels)
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

vlg_arraylist_t *vlog_level_list_new(void)
{
	vlg_arraylist_t *levels;

	levels = vlg_arraylist_new((vlg_arraylist_del_fn)vlog_level_del);
	if (!levels) {
		vlg_error("vlg_arraylist_new fail");
		return NULL;
	}

	if (vlog_level_list_set_default(levels)) {
		vlg_error("vlog_level_set_default fail");
		goto err;
	}

	//vlog_level_list_profile(levels, vlg_DEBUG);
	return levels;
err:
	vlg_arraylist_del(levels);
	return NULL;
}

int vlog_level_list_set(vlg_arraylist_t *levels, char *line)
{
	vlog_level_t *a_level;

	a_level = vlog_level_new(line);
	if (!a_level) {
		vlg_error("vlog_level_new fail");
		return -1;
	}

	if (vlg_arraylist_set(levels, a_level->int_level, a_level)) {
		vlg_error("vlg_arraylist_set fail");
		goto err;
	}

	return 0;
err:
	vlg_error("line[%s]", line);
	vlog_level_del(a_level);
	return -1;
}

vlog_level_t *vlog_level_list_get(vlg_arraylist_t *levels, int l)
{
	vlog_level_t *a_level;

#if 0
	if ((l <= 0) || (l > 254)) {
		/* illegal input from vlog() */
		vlg_error("l[%d] not in (0,254), set to UNKOWN", l);
		l = 254;
	}
#endif

	a_level = vlg_arraylist_get(levels, l);
	if (a_level) {
		return a_level;
	} else {
		/* empty slot */
		vlg_error("l[%d] not in (0,254), or has no level defined,"
			"see configure file define, set to UNKOWN", l);
		return vlg_arraylist_get(levels, 254);
	}
}

int vlog_level_list_atoi(vlg_arraylist_t *levels, char *str)
{
	int i;
	vlog_level_t *a_level;

	if (str == NULL || *str == '\0') {
		vlg_error("str is [%s], can't find level", str);
		return -1;
	}

	vlg_arraylist_foreach(levels, i, a_level) {
		if (a_level && STRICMP(str, ==, a_level->str_uppercase)) {
			return i;
		}
	}

	vlg_error("str[%s] can't found in level list", str);
	return -1;
}

