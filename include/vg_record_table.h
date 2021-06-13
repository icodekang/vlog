#ifndef __vlog_record_table_h
#define __vlog_record_table_h

#include "vg_defs.h"
#include "vg_record.h"

vg_hashtable_t *vlog_record_table_new(void);
void vlog_record_table_del(vg_hashtable_t * records);
void vlog_record_table_profile(vg_hashtable_t * records, int flag);

#endif
