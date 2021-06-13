#ifndef __vg_arraylist_h
#define __vg_arraylist_h

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (*vg_arraylist_del_fn) (void *data);
typedef int (*vg_arraylist_cmp_fn) (void *data1, void *data2);

/* make vg_arraylist_foreach speed up, so keep struct defination here */
typedef struct {
	void **array;
	int len;
	int size;
	vg_arraylist_del_fn del;
} vg_arraylist_t;

vg_arraylist_t *vg_arraylist_new(vg_arraylist_del_fn del);
void vg_arraylist_del(vg_arraylist_t * a_list);

int vg_arraylist_set(vg_arraylist_t * a_list, int i, void *data);
int vg_arraylist_add(vg_arraylist_t * a_list, void *data);
int vg_arraylist_sortadd(vg_arraylist_t * a_list, vg_arraylist_cmp_fn cmp,
			 void *data);

#define vg_arraylist_len(a_list)  (a_list->len)

#define vg_arraylist_get(a_list, i) \
	 ((i >= a_list->len) ? NULL : a_list->array[i])

#define vg_arraylist_foreach(a_list, i, a_unit) \
	for(i = 0, a_unit = a_list->array[0]; (i < a_list->len) && (a_unit = a_list->array[i], 1) ; i++)

#endif
