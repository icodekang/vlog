#ifndef __vlg_hashtalbe_h
#define __vlg_hashtalbe_h

#include <stdlib.h>

typedef struct vlg_hashtable_entry_s {
	unsigned int hash_key;
	void *key;
	void *value;
	struct vlg_hashtable_entry_s *prev;
	struct vlg_hashtable_entry_s *next;
} vlg_hashtable_entry_t;

typedef struct vlg_hashtable_s vlg_hashtable_t;

typedef unsigned int (*vlg_hashtable_hash_fn) (const void *key);
typedef int (*vlg_hashtable_equal_fn) (const void *key1, const void *key2);
typedef void (*vlg_hashtable_del_fn) (void *kv);

vlg_hashtable_t *vlg_hashtable_new(size_t a_size,
				 vlg_hashtable_hash_fn hash_fn,
				 vlg_hashtable_equal_fn equal_fn,
				 vlg_hashtable_del_fn key_del_fn,
				 vlg_hashtable_del_fn value_del_fn);

void vlg_hashtable_del(vlg_hashtable_t * a_table);
void vlg_hashtable_clean(vlg_hashtable_t * a_table);
int vlg_hashtable_put(vlg_hashtable_t * a_table, void *a_key, void *a_value);
vlg_hashtable_entry_t *vlg_hashtable_get_entry(vlg_hashtable_t * a_table, const void *a_key);
void *vlg_hashtable_get(vlg_hashtable_t * a_table, const void *a_key);
void vlg_hashtable_remove(vlg_hashtable_t * a_table, const void *a_key);
vlg_hashtable_entry_t *vlg_hashtable_begin(vlg_hashtable_t * a_table);
vlg_hashtable_entry_t *vlg_hashtable_next(vlg_hashtable_t * a_table, vlg_hashtable_entry_t * a_entry);

#define vlg_hashtable_foreach(a_table, a_entry) \
for(a_entry = vlg_hashtable_begin(a_table); a_entry; a_entry = vlg_hashtable_next(a_table, a_entry))

unsigned int vlg_hashtable_str_hash(const void *str);
int vlg_hashtable_str_equal(const void *key1, const void *key2);

#endif
