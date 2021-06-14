#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vlg_defs.h"
#include "vlg_record_table.h"

void vlog_record_table_profile(vlg_hashtable_t * records, int flag)
{
	vlg_hashtable_entry_t *a_entry;
	vlog_record_t *a_record;

	vlg_assert(records,);
	vlg_profile(flag, "-record_table[%p]-", records);
	vlg_hashtable_foreach(records, a_entry) {
		a_record = (vlog_record_t *) a_entry->value;
		vlog_record_profile(a_record, flag);
	}
	return;
}

/*******************************************************************************/

void vlog_record_table_del(vlg_hashtable_t * records)
{
	vlg_assert(records,);
	vlg_hashtable_del(records);
	vlg_debug("vlog_record_table_del[%p]", records);
	return;
}

vlg_hashtable_t *vlog_record_table_new(void)
{
	vlg_hashtable_t *records;

	records = vlg_hashtable_new(20,
			 (vlg_hashtable_hash_fn) vlg_hashtable_str_hash,
			 (vlg_hashtable_equal_fn) vlg_hashtable_str_equal,
			 NULL, (vlg_hashtable_del_fn) vlog_record_del);
	if (!records) {
		vlg_error("vlg_hashtable_new fail");
		return NULL;
	} else {
		vlog_record_table_profile(records, vlg_DEBUG);
		return records;
	}
}
/*******************************************************************************/
