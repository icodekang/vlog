#ifndef __vlog_level_list_h
#define __vlog_level_list_h

#include "vlg_defs.h"
#include "vlg_level.h"

vlg_arraylist_t *vlog_level_list_new(void);
void vlog_level_list_del(vlg_arraylist_t *levels);
void vlog_level_list_profile(vlg_arraylist_t *levels, int flag);

/* conf init use, slow */
/* if l is wrong or str=="", return -1 */
int vlog_level_list_set(vlg_arraylist_t *levels, char *line);

/* spec ouput use, fast */
/* rule output use, fast */
/* if not found, return levels[254] */
vlog_level_t *vlog_level_list_get(vlg_arraylist_t *levels, int l);

/* rule init use, slow */
/* if not found, return -1 */
int vlog_level_list_atoi(vlg_arraylist_t *levels, char *str);


#endif
