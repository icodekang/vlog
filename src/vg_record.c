#include "errno.h"
#include "vg_defs.h"
#include "vg_record.h"

void vlog_record_profile(vlog_record_t *a_record, int flag)
{
	vg_assert(a_record,);
	vg_profile(flag, "--record:[%p][%s:%p]--", a_record, a_record->name,  a_record->output);
	return;
}

void vlog_record_del(vlog_record_t *a_record)
{
	vg_assert(a_record,);
	vg_debug("vlog_record_del[%p]", a_record);
    free(a_record);
	return;
}

vlog_record_t *vlog_record_new(const char *name, vlog_record_fn output)
{
	vlog_record_t *a_record;

	vg_assert(name, NULL);
	vg_assert(output, NULL);

	a_record = calloc(1, sizeof(vlog_record_t));
	if (!a_record) {
		vg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (strlen(name) > sizeof(a_record->name) - 1) {
		vg_error("name[%s] is too long", name);
		goto err;
	}

	strcpy(a_record->name, name);
	a_record->output = output;

	vlog_record_profile(a_record, vg_DEBUG);
	return a_record;
err:
	vlog_record_del(a_record);
	return NULL;
}
