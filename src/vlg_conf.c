#include "vlg_fmacros.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "vlg_conf.h"
#include "vlg_rule.h"
#include "vlg_format.h"
#include "vlg_level_list.h"
#include "vlg_rotater.h"
#include "vlg_defs.h"

#define vlog_CONF_DEFAULT_FORMAT "default = \"%D %V [%p:%F:%L] %m%n\""
#define vlog_CONF_DEFAULT_RULE "*.*        >stdout"
#define vlog_CONF_DEFAULT_BUF_SIZE_MIN 1024
#define vlog_CONF_DEFAULT_BUF_SIZE_MAX (2 * 1024 * 1024)
#define vlog_CONF_DEFAULT_FILE_PERMS 0600
#define vlog_CONF_DEFAULT_RELOAD_CONF_PERIOD 0
#define vlog_CONF_DEFAULT_FSYNC_PERIOD 0
#define vlog_CONF_BACKUP_ROTATE_LOCK_FILE "/tmp/vlog.lock"

void vlog_conf_profile(vlog_conf_t * a_conf, int flag)
{
	int i;
	vlog_rule_t *a_rule;
	vlog_format_t *a_format;

	vlg_assert(a_conf,);
	vlg_profile(flag, "-conf[%p]-", a_conf);
	vlg_profile(flag, "--global--");
	vlg_profile(flag, "---file[%s],mtime[%s]---", a_conf->file, a_conf->mtime);
	vlg_profile(flag, "---in-memory conf[%s]---", a_conf->cfg_ptr);
	vlg_profile(flag, "---strict init[%d]---", a_conf->strict_init);
	vlg_profile(flag, "---buffer min[%ld]---", a_conf->buf_size_min);
	vlg_profile(flag, "---buffer max[%ld]---", a_conf->buf_size_max);
	if (a_conf->default_format) {
		vlg_profile(flag, "---default_format---");
		vlog_format_profile(a_conf->default_format, flag);
	}
	vlg_profile(flag, "---file perms[0%o]---", a_conf->file_perms);
	vlg_profile(flag, "---reload conf period[%ld]---", a_conf->reload_conf_period);
	vlg_profile(flag, "---fsync period[%ld]---", a_conf->fsync_period);

	vlg_profile(flag, "---rotate lock file[%s]---", a_conf->rotate_lock_file);
	if (a_conf->rotater) vlog_rotater_profile(a_conf->rotater, flag);

	if (a_conf->levels) vlog_level_list_profile(a_conf->levels, flag);

	if (a_conf->formats) {
		vlg_profile(flag, "--format list[%p]--", a_conf->formats);
		vlg_arraylist_foreach(a_conf->formats, i, a_format) {
			vlog_format_profile(a_format, flag);
		}
	}

	if (a_conf->rules) {
		vlg_profile(flag, "--rule_list[%p]--", a_conf->rules);
		vlg_arraylist_foreach(a_conf->rules, i, a_rule) {
			vlog_rule_profile(a_rule, flag);
		}
	}

	return;
}

void vlog_conf_del(vlog_conf_t * a_conf)
{
	vlg_assert(a_conf,);
	if (a_conf->rotater) vlog_rotater_del(a_conf->rotater);
	if (a_conf->levels) vlog_level_list_del(a_conf->levels);
	if (a_conf->default_format) vlog_format_del(a_conf->default_format);
	if (a_conf->formats) vlg_arraylist_del(a_conf->formats);
	if (a_conf->rules) vlg_arraylist_del(a_conf->rules);
	free(a_conf);
	vlg_debug("vlog_conf_del[%p]");
	return;
}

static int vlog_conf_build_without_file(vlog_conf_t * a_conf);
static int vlog_conf_build_with_file(vlog_conf_t * a_conf);
static int vlog_conf_build_with_in_memory(vlog_conf_t * a_conf);

enum{
	NO_CFG,
	FILE_CFG,
	IN_MEMORY_CFG
};

vlog_conf_t *vlog_conf_new(const char *config)
{
	int nwrite = 0;
	int cfg_source = 0;
	vlog_conf_t *a_conf = NULL;

	a_conf = calloc(1, sizeof(vlog_conf_t));
	if (!a_conf) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	// Find content of pointer. If it starts with '[' then content are configurations.
	if (config && config[0] != '\0' && config[0] != '[') {
		nwrite = snprintf(a_conf->file, sizeof(a_conf->file), "%s", config);
		cfg_source = FILE_CFG;
	} else if (getenv("vlog_CONF_PATH") != NULL) {
		nwrite = snprintf(a_conf->file, sizeof(a_conf->file), "%s", getenv("vlog_CONF_PATH"));
		cfg_source = FILE_CFG;
	} else if (config[0]=='[') {
		memset(a_conf->file, 0x00, sizeof(a_conf->file));
		nwrite = snprintf(a_conf->cfg_ptr, sizeof(a_conf->cfg_ptr), "%s", config);
		cfg_source = IN_MEMORY_CFG;
		if (nwrite < 0 || nwrite >= sizeof(a_conf->file)) {
			vlg_error("not enough space for configurations, nwrite=[%d], errno[%d]", nwrite, errno);
			goto err;
		}
	} else {
		memset(a_conf->file, 0x00, sizeof(a_conf->file));
		cfg_source = NO_CFG;
	}
	if (nwrite < 0 || (nwrite >= sizeof(a_conf->file) && cfg_source == FILE_CFG)) {
		vlg_error("not enough space for path name, nwrite=[%d], errno[%d]", nwrite, errno);
		goto err;
	}

	/* set default configuration start */
	a_conf->strict_init = 1;
	a_conf->buf_size_min = vlog_CONF_DEFAULT_BUF_SIZE_MIN;
	a_conf->buf_size_max = vlog_CONF_DEFAULT_BUF_SIZE_MAX;
	if (cfg_source == FILE_CFG) {
		/* configure file as default lock file */
		strcpy(a_conf->rotate_lock_file, a_conf->file);
	} else {
		strcpy(a_conf->rotate_lock_file, vlog_CONF_BACKUP_ROTATE_LOCK_FILE);
	}
	strcpy(a_conf->default_format_line, vlog_CONF_DEFAULT_FORMAT);
	a_conf->file_perms = vlog_CONF_DEFAULT_FILE_PERMS;
	a_conf->reload_conf_period = vlog_CONF_DEFAULT_RELOAD_CONF_PERIOD;
	a_conf->fsync_period = vlog_CONF_DEFAULT_FSYNC_PERIOD;
	/* set default configuration end */

	a_conf->levels = vlog_level_list_new();
	if (!a_conf->levels) {
		vlg_error("vlog_level_list_new fail");
		goto err;
	}

	a_conf->formats = vlg_arraylist_new((vlg_arraylist_del_fn) vlog_format_del);
	if (!a_conf->formats) {
		vlg_error("vlg_arraylist_new fail");
		goto err;
	}

	a_conf->rules = vlg_arraylist_new((vlg_arraylist_del_fn) vlog_rule_del);
	if (!a_conf->rules) {
		vlg_error("init rule_list fail");
		goto err;
	}

	if (cfg_source == FILE_CFG) {
		if (vlog_conf_build_with_file(a_conf)) {
			vlg_error("vlog_conf_build_with_file fail");
			goto err;
		}
	} else if (cfg_source == IN_MEMORY_CFG) {
		if(vlog_conf_build_with_in_memory(a_conf)){
			vlg_error("vlog_conf_build_with_in_memory fail");
			goto err;
		}
	} else {
		if (vlog_conf_build_without_file(a_conf)) {
			vlg_error("vlog_conf_build_without_file fail");
			goto err;
		}
	}

	vlog_conf_profile(a_conf, vlg_DEBUG);
	return a_conf;
err:
	vlog_conf_del(a_conf);
	return NULL;
}

static int vlog_conf_build_without_file(vlog_conf_t * a_conf)
{
	vlog_rule_t *default_rule;

	a_conf->default_format = vlog_format_new(a_conf->default_format_line, &(a_conf->time_cache_count));
	if (!a_conf->default_format) {
		vlg_error("vlog_format_new fail");
		return -1;
	}

	a_conf->rotater = vlog_rotater_new(a_conf->rotate_lock_file);
	if (!a_conf->rotater) {
		vlg_error("vlog_rotater_new fail");
		return -1;
	}

	default_rule = vlog_rule_new(
			vlog_CONF_DEFAULT_RULE,
			a_conf->levels,
			a_conf->default_format,
			a_conf->formats,
			a_conf->file_perms,
			a_conf->fsync_period,
			&(a_conf->time_cache_count));
	if (!default_rule) {
		vlg_error("vlog_rule_new fail");
		return -1;
	}

	/* add default rule */
	if (vlg_arraylist_add(a_conf->rules, default_rule)) {
		vlog_rule_del(default_rule);
		vlg_error("vlg_arraylist_add fail");
		return -1;
	}

	return 0;
}

static int vlog_conf_parse_line(vlog_conf_t * a_conf, char *line, int *section);

static int vlog_conf_build_with_file(vlog_conf_t * a_conf)
{
	int rc = 0;
	struct vlog_stat a_stat;
	struct tm local_time;
	FILE *fp = NULL;

	char line[MAXLEN_CFG_LINE + 1];
	size_t line_len;
	char *pline = NULL;
	char *p = NULL;
	int line_no = 0;
	int i = 0;
	int in_quotation = 0;

	int section = 0;
	/* [global:1] [levels:2] [formats:3] [rules:4] */

	if (lstat(a_conf->file, &a_stat)) {
		vlg_error("lstat conf file[%s] fail, errno[%d]", a_conf->file,
			 errno);
		return -1;
	}
	localtime_r(&(a_stat.st_mtime), &local_time);
	strftime(a_conf->mtime, sizeof(a_conf->mtime), "%F %T", &local_time);

	if ((fp = fopen(a_conf->file, "r")) == NULL) {
		vlg_error("open configure file[%s] fail", a_conf->file);
		return -1;
	}

	/* Now process the file.
	 */
	pline = line;
	memset(&line, 0x00, sizeof(line));
	while (fgets((char *)pline, sizeof(line) - (pline - line), fp) != NULL) {
		++line_no;
		line_len = strlen(pline);
		if (0 == line_len) {
			continue;
		}

		if (pline[line_len - 1] == '\n') {
			pline[line_len - 1] = '\0';
		}

		/* check for end-of-section, comments, strip off trailing
		 * spaces and newline character.
		 */
		p = pline;
		while (*p && isspace((int)*p))
			++p;
		if (*p == '\0' || *p == '#')
			continue;

		for (i = 0; p[i] != '\0'; ++i) {
			pline[i] = p[i];
		}
		pline[i] = '\0';

		for (p = pline + strlen(pline) - 1; isspace((int)*p); --p)
			/*EMPTY*/;

		if (*p == '\\') {
			if ((p - line) > MAXLEN_CFG_LINE - 30) {
				/* Oops the buffer is full - what now? */
				pline = line;
			} else {
				for (p--; isspace((int)*p); --p)
					/*EMPTY*/;
				p++;
				*p = 0;
				pline = p;
				continue;
			}
		} else
			pline = line;

		*++p = '\0';

		/* clean the tail comments start from # and not in quotation */
		in_quotation = 0;
		for (p = line; *p != '\0'; p++) {
			if (*p == '"') {
				in_quotation ^= 1;
				continue;
			}

			if (*p == '#' && !in_quotation) {
				*p = '\0';
				break;
			}
		}

		/* we now have the complete line,
		 * and are positioned at the first non-whitespace
		 * character. So let's process it
		 */
		rc = vlog_conf_parse_line(a_conf, line, &section);
		if (rc < 0) {
			vlg_error("parse configure file[%s]line_no[%ld] fail", a_conf->file, line_no);
			vlg_error("line[%s]", line);
			goto exit;
		} else if (rc > 0) {
			vlg_warn("parse configure file[%s]line_no[%ld] fail", a_conf->file, line_no);
			vlg_warn("line[%s]", line);
			vlg_warn("as strict init is set to false, ignore and go on");
			rc = 0;
			continue;
		}
	}

exit:
	fclose(fp);
	return rc;
}

static int vlog_conf_build_with_in_memory(vlog_conf_t * a_conf)
{
	int rc = 0;
	char line[MAXLEN_CFG_LINE + 1];
	char *pline = NULL;
	int section = 0;
	pline = line;
	memset(&line, 0x00, sizeof(line));
	pline = strtok((char *)a_conf->cfg_ptr, "\n");

	while (pline != NULL) {
		rc = vlog_conf_parse_line(a_conf, pline, &section);
		if (rc < 0) {
			vlg_error("parse in-memory configurations[%s] line [%s] fail", a_conf->cfg_ptr, pline);
			break;
		} else if (rc > 0) {
			vlg_error("parse in-memory configurations[%s] line [%s] fail", a_conf->cfg_ptr, pline);
			vlg_warn("as strict init is set to false, ignore and go on");
			rc = 0;
			continue;
		}
		pline = strtok(NULL, "\n");
	}
	return rc;
}
/* section [global:1] [levels:2] [formats:3] [rules:4] */
static int vlog_conf_parse_line(vlog_conf_t * a_conf, char *line, int *section)
{
	int nscan;
	int nread;
	char name[MAXLEN_CFG_LINE + 1];
	char word_1[MAXLEN_CFG_LINE + 1];
	char word_2[MAXLEN_CFG_LINE + 1];
	char word_3[MAXLEN_CFG_LINE + 1];
	char value[MAXLEN_CFG_LINE + 1];
	vlog_format_t *a_format = NULL;
	vlog_rule_t *a_rule = NULL;

	if (strlen(line) > MAXLEN_CFG_LINE) {
		vlg_error ("line_len[%ld] > MAXLEN_CFG_LINE[%ld], may cause overflow",
			strlen(line), MAXLEN_CFG_LINE);
		return -1;
	}

	/* get and set outer section flag, so it is a closure? haha */
	if (line[0] == '[') {
		int last_section = *section;
		nscan = sscanf(line, "[ %[^] \t]", name);
		if (STRCMP(name, ==, "global")) {
			*section = 1;
		} else if (STRCMP(name, ==, "levels")) {
			*section = 2;
		} else if (STRCMP(name, ==, "formats")) {
			*section = 3;
		} else if (STRCMP(name, ==, "rules")) {
			*section = 4;
		} else {
			vlg_error("wrong section name[%s]", name);
			return -1;
		}
		/* check the sequence of section, must increase */
		if (last_section >= *section) {
			vlg_error("wrong sequence of section, must follow global->levels->formats->rules");
			return -1;
		}

		if (*section == 4) {
			if (a_conf->reload_conf_period != 0
				&& a_conf->fsync_period >= a_conf->reload_conf_period) {
				/* as all rule will be rebuilt when conf is reload,
				 * so fsync_period > reload_conf_period will never
				 * cause rule to fsync it's file.
				 * fsync_period will be meaningless and down speed,
				 * so make it zero.
				 */
				vlg_warn("fsync_period[%ld] >= reload_conf_period[%ld],"
					"set fsync_period to zero");
				a_conf->fsync_period = 0;
			}

			/* now build rotater and default_format
			 * from the unchanging global setting,
			 * for vlog_rule_new() */
			a_conf->rotater = vlog_rotater_new(a_conf->rotate_lock_file);
			if (!a_conf->rotater) {
				vlg_error("vlog_rotater_new fail");
				return -1;
			}

			a_conf->default_format = vlog_format_new(a_conf->default_format_line,
							&(a_conf->time_cache_count));
			if (!a_conf->default_format) {
				vlg_error("vlog_format_new fail");
				return -1;
			}
		}
		return 0;
	}

	/* process detail */
	switch (*section) {
	case 1:
		memset(name, 0x00, sizeof(name));
		memset(value, 0x00, sizeof(value));
		nscan = sscanf(line, " %[^=]= %s ", name, value);
		if (nscan != 2) {
			vlg_error("sscanf [%s] fail, name or value is null", line);
			return -1;
		}

		memset(word_1, 0x00, sizeof(word_1));
		memset(word_2, 0x00, sizeof(word_2));
		memset(word_3, 0x00, sizeof(word_3));
		nread = 0;
		nscan = sscanf(name, "%s%n%s%s", word_1, &nread, word_2, word_3);

		if (STRCMP(word_1, ==, "strict") && STRCMP(word_2, ==, "init")) {
			/* if environment variable vlog_STRICT_INIT is set
			 * then always make it strict
			 */
			if (STRICMP(value, ==, "false") && !getenv("vlog_STRICT_INIT")) {
				a_conf->strict_init = 0;
			} else {
				a_conf->strict_init = 1;
			}
		} else if (STRCMP(word_1, ==, "buffer") && STRCMP(word_2, ==, "min")) {
			a_conf->buf_size_min = vlg_parse_byte_size(value);
		} else if (STRCMP(word_1, ==, "buffer") && STRCMP(word_2, ==, "max")) {
			a_conf->buf_size_max = vlg_parse_byte_size(value);
		} else if (STRCMP(word_1, ==, "file") && STRCMP(word_2, ==, "perms")) {
			sscanf(value, "%o", &(a_conf->file_perms));
		} else if (STRCMP(word_1, ==, "rotate") &&
				STRCMP(word_2, ==, "lock") && STRCMP(word_3, ==, "file")) {
			/* may overwrite the inner default value, or last value */
			if (STRCMP(value, ==, "self")) {
				strcpy(a_conf->rotate_lock_file, a_conf->file);
			} else {
				strcpy(a_conf->rotate_lock_file, value);
			}
		} else if (STRCMP(word_1, ==, "default") && STRCMP(word_2, ==, "format")) {
			/* so the input now is [format = "xxyy"], fit format's style */
			strcpy(a_conf->default_format_line, line + nread);
		} else if (STRCMP(word_1, ==, "reload") &&
				STRCMP(word_2, ==, "conf") && STRCMP(word_3, ==, "period")) {
			a_conf->reload_conf_period = vlg_parse_byte_size(value);
		} else if (STRCMP(word_1, ==, "fsync") && STRCMP(word_2, ==, "period")) {
			a_conf->fsync_period = vlg_parse_byte_size(value);
		} else {
			vlg_error("name[%s] is not any one of global options", name);
			if (a_conf->strict_init) return -1;
		}
		break;
	case 2:
		if (vlog_level_list_set(a_conf->levels, line)) {
			vlg_error("vlog_level_list_set fail");
			if (a_conf->strict_init) return -1;
		}
		break;
	case 3:
		a_format = vlog_format_new(line, &(a_conf->time_cache_count));
		if (!a_format) {
			vlg_error("vlog_format_new fail [%s]", line);
			if (a_conf->strict_init) return -1;
			else break;
		}
		if (vlg_arraylist_add(a_conf->formats, a_format)) {
			vlog_format_del(a_format);
			vlg_error("vlg_arraylist_add fail");
			return -1;
		}
		break;
	case 4:
		a_rule = vlog_rule_new(line,
			a_conf->levels,
			a_conf->default_format,
			a_conf->formats,
			a_conf->file_perms,
			a_conf->fsync_period,
			&(a_conf->time_cache_count));

		if (!a_rule) {
			vlg_error("vlog_rule_new fail [%s]", line);
			if (a_conf->strict_init) return -1;
			else break;
		}
		if (vlg_arraylist_add(a_conf->rules, a_rule)) {
			vlog_rule_del(a_rule);
			vlg_error("vlg_arraylist_add fail");
			return -1;
		}
		break;
	default:
		vlg_error("not in any section");
		return -1;
	}

	return 0;
}