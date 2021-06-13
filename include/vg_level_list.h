#ifndef __vlog_level_list_h
#define __vlog_level_list_h

#include "vg_defs.h"
#include "vg_level.h"

vg_arraylist_t *vlog_level_list_new(void);
void vlog_level_list_del(vg_arraylist_t *levels);
void vlog_level_list_profile(vg_arraylist_t *levels, int flag);

/* conf init use, slow */
/* if l is wrong or str=="", return -1 */
int vlog_level_list_set(vg_arraylist_t *levels, char *line);

/* spec ouput use, fast */
/* rule output use, fast */
/* if not found, return levels[254] */
vlog_level_t *vlog_level_list_get(vg_arraylist_t *levels, int l);

/* rule init use, slow */
/* if not found, return -1 */
int vlog_level_list_atoi(vg_arraylist_t *levels, char *str);


#endif
