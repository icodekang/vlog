#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "vlg_hashtable.h"

void myfree(void *kv)
{
}

int main(void)
{
	vlg_hashtable_t *a_table;
	vlg_hashtable_entry_t *a_entry;

	a_table = vlg_hashtable_new(20,
		vlg_hashtable_str_hash,
		vlg_hashtable_str_equal,
		myfree, myfree);

	vlg_hashtable_put(a_table, "aaa", "bnbb");
	vlg_hashtable_put(a_table, "bbb", "bnbb");
	vlg_hashtable_put(a_table, "ccc", "bnbb");

	vlg_hashtable_put(a_table, "aaa", "123");

	vlg_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}

	printf("getv[%s]\n", (char*)vlg_hashtable_get(a_table, "ccc"));

	vlg_hashtable_remove(a_table, "ccc");

	vlg_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}


	vlg_hashtable_remove(a_table, NULL);
	vlg_hashtable_del(NULL);

	vlg_hashtable_del(a_table);
	return 0;
}

