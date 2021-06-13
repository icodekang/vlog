#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "vg_defs.h"

vg_arraylist_t *vg_arraylist_new(vg_arraylist_del_fn del)
{
	vg_arraylist_t *a_list;

	a_list = (vg_arraylist_t *) calloc(1, sizeof(vg_arraylist_t));
	if (!a_list) {
		vg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}
	a_list->size = ARRAY_LIST_DEFAULT_SIZE;
	a_list->len = 0;

	/* this could be NULL */
	a_list->del = del;
	a_list->array = (void **)calloc(a_list->size, sizeof(void *));
	if (!a_list->array) {
		vg_error("calloc fail, errno[%d]", errno);
		free(a_list);
		return NULL;
	}

	return a_list;
}

void vg_arraylist_del(vg_arraylist_t * a_list)
{
	int i;

	if (!a_list)
		return;
	if (a_list->del) {
		for (i = 0; i < a_list->len; i++) {
			if (a_list->array[i])
				a_list->del(a_list->array[i]);
		}
	}
	if (a_list->array)
		free(a_list->array);
	free(a_list);
	return;
}

static int vg_arraylist_expand_inner(vg_arraylist_t * a_list, int max)
{
	void *tmp;
	int new_size;
	int diff_size;

	new_size = vg_max(a_list->size * 2, max);
	tmp = realloc(a_list->array, new_size * sizeof(void *));
	if (!tmp) {
		vg_error("realloc fail, errno[%d]", errno);
		return -1;
	}
	a_list->array = (void **)tmp;
	diff_size = new_size - a_list->size;
	if (diff_size) memset(a_list->array + a_list->size, 0x00, diff_size * sizeof(void *));
	a_list->size = new_size;
	return 0;
}

int vg_arraylist_set(vg_arraylist_t * a_list, int idx, void *data)
{
	if (idx > a_list->size - 1) {
		if (vg_arraylist_expand_inner(a_list, idx)) {
			vg_error("expand_internal fail");
			return -1;
		}
	}
	if (a_list->array[idx] && a_list->del) a_list->del(a_list->array[idx]);
	a_list->array[idx] = data;
	if (a_list->len <= idx)
		a_list->len = idx + 1;
	return 0;
}

int vg_arraylist_add(vg_arraylist_t * a_list, void *data)
{
	return vg_arraylist_set(a_list, a_list->len, data);
}

/* assum idx < len */
static int vg_arraylist_insert_inner(vg_arraylist_t * a_list, int idx,
				     void *data)
{
	if (a_list->array[idx] == NULL) {
		a_list->array[idx] = data;
		return 0;
	}
	if (a_list->len > a_list->size - 1) {
		if (vg_arraylist_expand_inner(a_list, 0)) {
			vg_error("expand_internal fail");
			return -1;
		}
	}
	memmove(a_list->array + idx + 1, a_list->array + idx,
		(a_list->len - idx) * sizeof(void *));
	a_list->array[idx] = data;
	a_list->len++;
	return 0;
}

int vg_arraylist_sortadd(vg_arraylist_t * a_list, vg_arraylist_cmp_fn cmp,
			 void *data)
{
	int i;

	for (i = 0; i < a_list->len; i++) {
		if ((*cmp) (a_list->array[i], data) > 0)
			break;
	}

	if (i == a_list->len)
		return vg_arraylist_add(a_list, data);
	else
		return vg_arraylist_insert_inner(a_list, i, data);
}
