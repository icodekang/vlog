#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "syslog.h"

#include "vlg_defs.h"
#include "vlg_level.h"

void vlog_level_profile(vlog_level_t *a_level, int flag)
{
	vlg_assert(a_level,);
	vlg_profile(flag, "---level[%p][%d,%s,%s,%d,%d]---",
		a_level,
		a_level->int_level,
		a_level->str_uppercase,
		a_level->str_lowercase,
		(int) a_level->str_len,
		a_level->syslog_level);
	return;
}

/*******************************************************************************/
void vlog_level_del(vlog_level_t *a_level)
{
	vlg_assert(a_level,);
	vlg_debug("vlog_level_del[%p]", a_level);
    free(a_level);
	return;
}

static int syslog_level_atoi(char *str)
{
	/* guess no unix system will choose -187
	 * as its syslog level, so it is a safe return value
	 */
	vlg_assert(str, -187);

	if (STRICMP(str, ==, "LOG_EMERG"))
		return LOG_EMERG;
	if (STRICMP(str, ==, "LOG_ALERT"))
		return LOG_ALERT;
	if (STRICMP(str, ==, "LOG_CRIT"))
		return LOG_CRIT;
	if (STRICMP(str, ==, "LOG_ERR"))
		return LOG_ERR;
	if (STRICMP(str, ==, "LOG_WARNING"))
		return LOG_WARNING;
	if (STRICMP(str, ==, "LOG_NOTICE"))
		return LOG_NOTICE;
	if (STRICMP(str, ==, "LOG_INFO"))
		return LOG_INFO;
	if (STRICMP(str, ==, "LOG_DEBUG"))
		return LOG_DEBUG;

	vlg_error("wrong syslog level[%s]", str);
	return -187;
}

/* line: TRACE = 10, LOG_ERR */
vlog_level_t *vlog_level_new(char *line)
{
	vlog_level_t *a_level = NULL;
	int i;
	int nscan;
	char str[MAXLEN_CFG_LINE + 1];
	int l = 0;
	char sl[MAXLEN_CFG_LINE + 1];

	vlg_assert(line, NULL);

	memset(str, 0x00, sizeof(str));
	memset(sl, 0x00, sizeof(sl));

	nscan = sscanf(line, " %[^= \t] = %d ,%s", str, &l, sl);
	if (nscan < 2) {
		vlg_error("level[%s], syntax wrong", line);
		return NULL;
	}

	/* check level and str */
	if ((l < 0) || (l > 255)) {
		vlg_error("l[%d] not in [0,255], wrong", l);
		return NULL;
	}

	if (str[0] == '\0') {
		vlg_error("str[0] = 0");
		return NULL;
	}

	a_level = calloc(1, sizeof(vlog_level_t));
	if (!a_level) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_level->int_level = l;

	/* fill syslog level */
	if (sl[0] == '\0') {
		a_level->syslog_level = LOG_DEBUG;
	} else {
		a_level->syslog_level = syslog_level_atoi(sl);
		if (a_level->syslog_level == -187) {
			vlg_error("syslog_level_atoi fail");
			goto err;
		}
	}

	/* strncpy and toupper(str)  */
	for (i = 0; (i < sizeof(a_level->str_uppercase) - 1) && str[i] != '\0'; i++) {
		(a_level->str_uppercase)[i] = toupper(str[i]);
		(a_level->str_lowercase)[i] = tolower(str[i]);
	}

	if (str[i] != '\0') {
		/* overflow */
		vlg_error("not enough space for str, str[%s] > %d", str, i);
		goto err;
	} else {
		(a_level->str_uppercase)[i] = '\0';
		(a_level->str_lowercase)[i] = '\0';
	}

	a_level->str_len = i;

	//vlog_level_profile(a_level, vlg_DEBUG);
	return a_level;
err:
	vlg_error("line[%s]", line);
	vlog_level_del(a_level);
	return NULL;
}
