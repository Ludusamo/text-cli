#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

#include "list.h"

typedef struct {
	const char *key;
	Value val;
} Keyval;

typedef struct {
	List a, b;
	size_t capacity;
	size_t size;
} Hashtable;

int ctor_hashtable(Hashtable *h);
int dtor_hashtable(Hashtable *h);

int set_hashtable(Hashtable *h, const char *key, Value val);
Value access_hashtable(const Hashtable *h, const char *key);
int delete_hashtable(Hashtable *h, const char *key);
int clear_hashtable(Hashtable *h);

int _rehash(Hashtable *h);
Keyval *_aux_set_hashtable(Hashtable *h, Keyval *new_pair,
                           List *list, uint64_t hash);

uint64_t hash1(const char *str);
uint64_t hash2(const char *str);

void next_iter_hashtable(Iter *self);
int done_iter_hashtable(const Iter *self);
Value val_iter_hashtable(const Iter *self);

int iter_hashtable(Iter *self, Hashtable *h);
void destroy_iter_hashtable(Iter *self);

#endif // HASHTABLE_H
