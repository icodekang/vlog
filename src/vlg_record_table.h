#ifndef __vlog_record_table_h
#define __vlog_record_table_h

#include "vlg_defs.h"
#include "vlg_record.h"

vlg_hashtable_t *vlog_record_table_new(void);
void vlog_record_table_del(vlg_hashtable_t * records);
void vlog_record_table_profile(vlg_hashtable_t * records, int flag);

#endif
