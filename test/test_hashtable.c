#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vg_hashtable.h"

void myfree(void *kv)
{
}

int main(void)
{
	vg_hashtable_t *a_table;
	vg_hashtable_entry_t *a_entry;

	a_table = vg_hashtable_new(20,
		vg_hashtable_str_hash,
		vg_hashtable_str_equal,
		myfree, myfree);

	vg_hashtable_put(a_table, "aaa", "bnbb");
	vg_hashtable_put(a_table, "bbb", "bnbb");
	vg_hashtable_put(a_table, "ccc", "bnbb");

	vg_hashtable_put(a_table, "aaa", "123");

	vg_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}

	printf("getv[%s]\n", (char*)vg_hashtable_get(a_table, "ccc"));

	vg_hashtable_remove(a_table, "ccc");

	vg_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}


	vg_hashtable_remove(a_table, NULL);
	vg_hashtable_del(NULL);

	vg_hashtable_del(a_table);
	return 0;
}

