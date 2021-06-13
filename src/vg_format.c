#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "vg_defs.h"
#include "vg_thread.h"
#include "vg_spec.h"
#include "vg_format.h"

void vlog_format_profile(vlog_format_t * a_format, int flag)
{

	vg_assert(a_format,);
	vg_profile(flag, "---format[%p][%s = %s(%p)]---",
		a_format,
		a_format->name,
		a_format->pattern,
		a_format->pattern_specs);

#if 0
	int i;
	vlog_spec_t *a_spec;
	vg_arraylist_foreach(a_format->pattern_specs, i, a_spec) {
		vlog_spec_profile(a_spec, flag);
	}
#endif

	return;
}

void vlog_format_del(vlog_format_t * a_format)
{
	vg_assert(a_format,);
	if (a_format->pattern_specs) {
		vg_arraylist_del(a_format->pattern_specs);
	}
	vg_debug("vlog_format_del[%p]", a_format);
    free(a_format);
	return;
}

vlog_format_t *vlog_format_new(char *line, int * time_cache_count)
{
	int nscan = 0;
	vlog_format_t *a_format = NULL;
	int nread = 0;
	const char *p_start;
	const char *p_end;
	char *p;
	char *q;
	vlog_spec_t *a_spec;

	vg_assert(line, NULL);

	a_format = calloc(1, sizeof(vlog_format_t));
	if (!a_format) {
		vg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	/* line         default = "%d(%F %X.%l) %-6V (%c:%F:%L) - %m%n"
	 * name         default
	 * pattern      %d(%F %X.%l) %-6V (%c:%F:%L) - %m%n
	 */
	memset(a_format->name, 0x00, sizeof(a_format->name));
	nread = 0;
	nscan = sscanf(line, " %[^= \t] = %n", a_format->name, &nread);
	if (nscan != 1) {
		vg_error("format[%s], syntax wrong", line);
		goto err;
	}

	if (*(line + nread) != '"') {
		vg_error("the 1st char of pattern is not \", line+nread[%s]", line+nread);
		goto err;
	}

	for (p = a_format->name; *p != '\0'; p++) {
		if ((!isalnum(*p)) && (*p != '_')) {
			vg_error("a_format->name[%s] character is not in [a-Z][0-9][_]", a_format->name);
			goto err;
		}
	}

	p_start = line + nread + 1;
	p_end = strrchr(p_start, '"');
	if (!p_end) {
		vg_error("there is no \" at end of pattern, line[%s]", line);
		goto err;
	}

	if (p_end - p_start > sizeof(a_format->pattern) - 1) {
		vg_error("pattern is too long");
		goto err;
	}
	memset(a_format->pattern, 0x00, sizeof(a_format->pattern));
	memcpy(a_format->pattern, p_start, p_end - p_start);

	if (vg_str_replace_env(a_format->pattern, sizeof(a_format->pattern))) {
		vg_error("vg_str_replace_env fail");
		goto err;
	}

	a_format->pattern_specs =
	    vg_arraylist_new((vg_arraylist_del_fn) vlog_spec_del);
	if (!(a_format->pattern_specs)) {
		vg_error("vg_arraylist_new fail");
		goto err;
	}

	for (p = a_format->pattern; *p != '\0'; p = q) {
		a_spec = vlog_spec_new(p, &q, time_cache_count);
		if (!a_spec) {
			vg_error("vlog_spec_new fail");
			goto err;
		}

		if (vg_arraylist_add(a_format->pattern_specs, a_spec)) {
			vlog_spec_del(a_spec);
			vg_error("vg_arraylist_add fail");
			goto err;
		}
	}

	vlog_format_profile(a_format, vg_DEBUG);
	return a_format;
err:
	vlog_format_del(a_format);
	return NULL;
}

/* return 0	success, or buf is full
 * return -1	fail
 */
int vlog_format_gen_msg(vlog_format_t * a_format, vlog_thread_t * a_thread)
{
	int i;
	vlog_spec_t *a_spec;

	vlog_buf_restart(a_thread->msg_buf);

	vg_arraylist_foreach(a_format->pattern_specs, i, a_spec) {
		if (vlog_spec_gen_msg(a_spec, a_thread) == 0) {
			continue;
		} else {
			return -1;
		}
	}

	return 0;
}
