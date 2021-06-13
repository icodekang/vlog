#ifndef __vg_hashtalbe_h
#define __vg_hashtalbe_h

#include <stdlib.h>

typedef struct vg_hashtable_entry_s {
	unsigned int hash_key;
	void *key;
	void *value;
	struct vg_hashtable_entry_s *prev;
	struct vg_hashtable_entry_s *next;
} vg_hashtable_entry_t;

typedef struct vg_hashtable_s vg_hashtable_t;

typedef unsigned int (*vg_hashtable_hash_fn) (const void *key);
typedef int (*vg_hashtable_equal_fn) (const void *key1, const void *key2);
typedef void (*vg_hashtable_del_fn) (void *kv);

vg_hashtable_t *vg_hashtable_new(size_t a_size,
				 vg_hashtable_hash_fn hash_fn,
				 vg_hashtable_equal_fn equal_fn,
				 vg_hashtable_del_fn key_del_fn,
				 vg_hashtable_del_fn value_del_fn);

void vg_hashtable_del(vg_hashtable_t * a_table);
void vg_hashtable_clean(vg_hashtable_t * a_table);
int vg_hashtable_put(vg_hashtable_t * a_table, void *a_key, void *a_value);
vg_hashtable_entry_t *vg_hashtable_get_entry(vg_hashtable_t * a_table, const void *a_key);
void *vg_hashtable_get(vg_hashtable_t * a_table, const void *a_key);
void vg_hashtable_remove(vg_hashtable_t * a_table, const void *a_key);
vg_hashtable_entry_t *vg_hashtable_begin(vg_hashtable_t * a_table);
vg_hashtable_entry_t *vg_hashtable_next(vg_hashtable_t * a_table, vg_hashtable_entry_t * a_entry);

#define vg_hashtable_foreach(a_table, a_entry) \
for(a_entry = vg_hashtable_begin(a_table); a_entry; a_entry = vg_hashtable_next(a_table, a_entry))

unsigned int vg_hashtable_str_hash(const void *str);
int vg_hashtable_str_equal(const void *key1, const void *key2);

#endif
