#ifndef __vlg_arraylist_h
#define __vlg_arraylist_h

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (*vlg_arraylist_del_fn) (void *data);
typedef int (*vlg_arraylist_cmp_fn) (void *data1, void *data2);

/* make vlg_arraylist_foreach speed up, so keep struct defination here */
typedef struct {
	void **array;
	int len;
	int size;
	vlg_arraylist_del_fn del;
} vlg_arraylist_t;

vlg_arraylist_t *vlg_arraylist_new(vlg_arraylist_del_fn del);
void vlg_arraylist_del(vlg_arraylist_t * a_list);

int vlg_arraylist_set(vlg_arraylist_t * a_list, int i, void *data);
int vlg_arraylist_add(vlg_arraylist_t * a_list, void *data);
int vlg_arraylist_sortadd(vlg_arraylist_t * a_list, vlg_arraylist_cmp_fn cmp,
			 void *data);

#define vlg_arraylist_len(a_list)  (a_list->len)

#define vlg_arraylist_get(a_list, i) \
	 ((i >= a_list->len) ? NULL : a_list->array[i])

#define vlg_arraylist_foreach(a_list, i, a_unit) \
	for(i = 0, a_unit = a_list->array[0]; (i < a_list->len) && (a_unit = a_list->array[i], 1) ; i++)

#endif
