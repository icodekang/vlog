#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "vlg_defs.h"
#include "vlg_hashtable.h"

struct vlg_hashtable_s {
	size_t nelem;

	vlg_hashtable_entry_t **tab;
	size_t tab_size;

	vlg_hashtable_hash_fn hash;
	vlg_hashtable_equal_fn equal;
	vlg_hashtable_del_fn key_del;
	vlg_hashtable_del_fn value_del;
};

vlg_hashtable_t *vlg_hashtable_new(size_t a_size,
				 vlg_hashtable_hash_fn hash,
				 vlg_hashtable_equal_fn equal,
				 vlg_hashtable_del_fn key_del,
				 vlg_hashtable_del_fn value_del)
{
	vlg_hashtable_t *a_table;

	a_table = calloc(1, sizeof(*a_table));
	if (!a_table) {
		vlg_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_table->tab = calloc(a_size, sizeof(*(a_table->tab)));
	if (!a_table->tab) {
		vlg_error("calloc fail, errno[%d]", errno);
		free(a_table);
		return NULL;
	}
	a_table->tab_size = a_size;

	a_table->nelem = 0;
	a_table->hash = hash;
	a_table->equal = equal;

	/* these two could be NULL */
	a_table->key_del = key_del;
	a_table->value_del = value_del;

	return a_table;
}

void vlg_hashtable_del(vlg_hashtable_t * a_table)
{
	size_t i;
	vlg_hashtable_entry_t *p;
	vlg_hashtable_entry_t *q;

	if (!a_table) {
		vlg_error("a_table[%p] is NULL, just do nothing", a_table);
		return;
	}

	for (i = 0; i < a_table->tab_size; i++) {
		for (p = (a_table->tab)[i]; p; p = q) {
			q = p->next;
			if (a_table->key_del) {
				a_table->key_del(p->key);
			}
			if (a_table->value_del) {
				a_table->value_del(p->value);
			}
			free(p);
		}
	}
	if (a_table->tab)
		free(a_table->tab);
	free(a_table);

	return;
}

void vlg_hashtable_clean(vlg_hashtable_t * a_table)
{
	size_t i;
	vlg_hashtable_entry_t *p;
	vlg_hashtable_entry_t *q;

	for (i = 0; i < a_table->tab_size; i++) {
		for (p = (a_table->tab)[i]; p; p = q) {
			q = p->next;
			if (a_table->key_del) {
				a_table->key_del(p->key);
			}
			if (a_table->value_del) {
				a_table->value_del(p->value);
			}
			free(p);
		}
		(a_table->tab)[i] = NULL;
	}
	a_table->nelem = 0;
	return;
}

static int vlg_hashtable_rehash(vlg_hashtable_t * a_table)
{
	size_t i;
	size_t j;
	size_t tab_size;
	vlg_hashtable_entry_t **tab;
	vlg_hashtable_entry_t *p;
	vlg_hashtable_entry_t *q;

	tab_size = 2 * a_table->tab_size;
	tab = calloc(tab_size, sizeof(*tab));
	if (!tab) {
		vlg_error("calloc fail, errno[%d]", errno);
		return -1;
	}

	for (i = 0; i < a_table->tab_size; i++) {
		for (p = (a_table->tab)[i]; p; p = q) {
			q = p->next;

			p->next = NULL;
			p->prev = NULL;
			j = p->hash_key % tab_size;
			if (tab[j]) {
				tab[j]->prev = p;
				p->next = tab[j];
			}
			tab[j] = p;
		}
	}
	free(a_table->tab);
	a_table->tab = tab;
	a_table->tab_size = tab_size;

	return 0;
}

vlg_hashtable_entry_t *vlg_hashtable_get_entry(vlg_hashtable_t * a_table, const void *a_key)
{
	unsigned int i;
	vlg_hashtable_entry_t *p;

	i = a_table->hash(a_key) % a_table->tab_size;
	for (p = (a_table->tab)[i]; p; p = p->next) {
		if (a_table->equal(a_key, p->key))
			return p;
	}

	return NULL;
}

void *vlg_hashtable_get(vlg_hashtable_t * a_table, const void *a_key)
{
	unsigned int i;
	vlg_hashtable_entry_t *p;

	i = a_table->hash(a_key) % a_table->tab_size;
	for (p = (a_table->tab)[i]; p; p = p->next) {
		if (a_table->equal(a_key, p->key))
			return p->value;
	}

	return NULL;
}

int vlg_hashtable_put(vlg_hashtable_t * a_table, void *a_key, void *a_value)
{
	int rc = 0;
	unsigned int i;
	vlg_hashtable_entry_t *p = NULL;

	i = a_table->hash(a_key) % a_table->tab_size;
	for (p = (a_table->tab)[i]; p; p = p->next) {
		if (a_table->equal(a_key, p->key))
			break;
	}

	if (p) {
		if (a_table->key_del) {
			a_table->key_del(p->key);
		}
		if (a_table->value_del) {
			a_table->value_del(p->value);
		}
		p->key = a_key;
		p->value = a_value;
		return 0;
	} else {
		if (a_table->nelem > a_table->tab_size * 1.3) {
			rc = vlg_hashtable_rehash(a_table);
			if (rc) {
				vlg_error("rehash fail");
				return -1;
			}
		}

		p = calloc(1, sizeof(*p));
		if (!p) {
			vlg_error("calloc fail, errno[%d]", errno);
			return -1;
		}

		p->hash_key = a_table->hash(a_key);
		p->key = a_key;
		p->value = a_value;
		p->next = NULL;
		p->prev = NULL;

		i = p->hash_key % a_table->tab_size;
		if ((a_table->tab)[i]) {
			(a_table->tab)[i]->prev = p;
			p->next = (a_table->tab)[i];
		}
		(a_table->tab)[i] = p;
		a_table->nelem++;
	}

	return 0;
}

void vlg_hashtable_remove(vlg_hashtable_t * a_table, const void *a_key)
{
	vlg_hashtable_entry_t *p;
	unsigned int i;

        if (!a_table || !a_key) {
		vlg_error("a_table[%p] or a_key[%p] is NULL, just do nothing", a_table, a_key);
		return;
        }

	i = a_table->hash(a_key) % a_table->tab_size;
	for (p = (a_table->tab)[i]; p; p = p->next) {
		if (a_table->equal(a_key, p->key))
			break;
	}

	if (!p) {
		vlg_error("p[%p] not found in hashtable", p);
		return;
	}

	if (a_table->key_del) {
		a_table->key_del(p->key);
	}
	if (a_table->value_del) {
		a_table->value_del(p->value);
	}

	if (p->next) {
		p->next->prev = p->prev;
	}
	if (p->prev) {
		p->prev->next = p->next;
	} else {
		unsigned int i;

		i = p->hash_key % a_table->tab_size;
		a_table->tab[i] = p->next;
	}

	free(p);
	a_table->nelem--;

	return;
}

vlg_hashtable_entry_t *vlg_hashtable_begin(vlg_hashtable_t * a_table)
{
	size_t i;
	vlg_hashtable_entry_t *p;

	for (i = 0; i < a_table->tab_size; i++) {
		for (p = (a_table->tab)[i]; p; p = p->next) {
			if (p)
				return p;
		}
	}

	return NULL;
}

vlg_hashtable_entry_t *vlg_hashtable_next(vlg_hashtable_t * a_table, vlg_hashtable_entry_t * a_entry)
{
	size_t i;
	size_t j;

	if (a_entry->next)
		return a_entry->next;

	i = a_entry->hash_key % a_table->tab_size;

	for (j = i + 1; j < a_table->tab_size; j++) {
		if ((a_table->tab)[j]) {
			return (a_table->tab)[j];
		}
	}

	return NULL;
}

/*******************************************************************************/

unsigned int vlg_hashtable_str_hash(const void *str)
{
	unsigned int h = 5381;
	const char *p = (const char *)str;

	while (*p != '\0')
		h = ((h << 5) + h) + (*p++); /* hash * 33 + c */

	return h;
}

int vlg_hashtable_str_equal(const void *key1, const void *key2)
{
	return (STRCMP((const char *)key1, ==, (const char *)key2));
}
