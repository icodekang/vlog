#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vg_defs.h"
#include "vg_record_table.h"

void vlog_record_table_profile(vg_hashtable_t * records, int flag)
{
	vg_hashtable_entry_t *a_entry;
	vlog_record_t *a_record;

	vg_assert(records,);
	vg_profile(flag, "-record_table[%p]-", records);
	vg_hashtable_foreach(records, a_entry) {
		a_record = (vlog_record_t *) a_entry->value;
		vlog_record_profile(a_record, flag);
	}
	return;
}

/*******************************************************************************/

void vlog_record_table_del(vg_hashtable_t * records)
{
	vg_assert(records,);
	vg_hashtable_del(records);
	vg_debug("vlog_record_table_del[%p]", records);
	return;
}

vg_hashtable_t *vlog_record_table_new(void)
{
	vg_hashtable_t *records;

	records = vg_hashtable_new(20,
			 (vg_hashtable_hash_fn) vg_hashtable_str_hash,
			 (vg_hashtable_equal_fn) vg_hashtable_str_equal,
			 NULL, (vg_hashtable_del_fn) vlog_record_del);
	if (!records) {
		vg_error("vg_hashtable_new fail");
		return NULL;
	} else {
		vlog_record_table_profile(records, vg_DEBUG);
		return records;
	}
}
/*******************************************************************************/
