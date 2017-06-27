#include "hashtable.h"

int ctor_hashtable(Hashtable *h) {
	ctor_list(&h->a);
	ctor_list(&h->b);
	h->capacity = 0;
	h->size = 0;
	return 1;
}

int dtor_hashtable(Hashtable *h) {
	clear_hashtable(h);
	return 1;
}

int set_hashtable(Hashtable *h, const char *key, void *val) {
	int cur = 1;
	Keyval *pair = malloc(sizeof(Keyval));
	pair->key = key;
	pair->val = val;
	for (size_t i = 0; i < h->a.length; i++) {
		uint64_t hash = 0;
		List *list = 0;
		if (cur) {
			hash = hash1(pair->key) % h->a.length;
			list = &h->a;
		} else {
			hash = hash2(pair->key) % h->b.length;
			list = &h->b;
		}
		Keyval *ret = _aux_set_hashtable(h, pair, list, hash);
		if (ret) {
			pair = ret;
			cur = !cur;
		} else {
			h->size++;
			return 1;
		}
	}
	_rehash(h);
	int ret = set_hashtable(h, pair->key, pair->val);
	free(pair);
	return ret;
}

Keyval *_aux_set_hashtable(Hashtable *h, Keyval *new_pair,
                           List *list, uint64_t hash) {
	Keyval *pair = access_list(list, hash);
	if (pair) {
		if (pair->key == new_pair->key) {
			h->size--;
			pair->val = new_pair->val;
			free(new_pair);
			return 0;
		}
	}
	set_list(list, hash, new_pair);
	return pair;
}

void *access_hashtable(const Hashtable *h, const char *key) {
	uint64_t h1 = hash1(key) % h->a.length;
	Keyval *pair = access_list(&h->a, h1);
	if (pair && pair->key == key) return pair->val;
	uint64_t h2 = hash2(key) % h->b.length;
	pair = access_list(&h->b, h2);
	if (pair && pair->key == key) return pair->val;
	return 0;
}

int _rehash(Hashtable *h) {
	List keyvals;
	ctor_list(&keyvals);
	for (size_t i = 0; i < h->capacity / 2; i++) {
		Keyval *a = access_list(&h->a, i);
		if (a) append_list(&keyvals, a);
		Keyval *b = access_list(&h->b, i);
		if (b) append_list(&keyvals, b);
	}
	if (h->capacity < 1) h->capacity = 1;
	clear_list(&h->a);
	clear_list(&h->b);
	resize_list(&h->a, h->capacity, 0);
	resize_list(&h->b, h->capacity, 0);
	h->size = 0;
	for (size_t i = 0; i < keyvals.length; i++) {
		Keyval *keyval = access_list(&keyvals, i);
		set_hashtable(h, keyval->key, keyval->val);
		free(keyval);
	}
	dtor_list(&keyvals);
	h->capacity *= 2;
	return 1;
}

int delete_hashtable(Hashtable *h, const char *key) {
	h->size--;
	uint64_t h1 = hash1(key) % h->a.length;
	Keyval *pair = access_list(&h->a, h1);
	if (pair->key == key) {
		set_list(&h->a, h1, 0);
		free(pair);
		return 1;
	}
	uint64_t h2 = hash2(key) % h->b.length;
	pair = access_list(&h->b, h2);
	if (pair->key == key) {
		set_list(&h->b, h2, 0);
		free(pair);
		return 1;
	}
	return 0;
}

int clear_hashtable(Hashtable *h) {
	for (size_t i = 0; i < h->capacity / 2; i++) {
		Keyval *a = access_list(&h->a, i);
		if (a) free(a);
		Keyval *b = access_list(&h->b, i);
		if (b) free(b);
	}
	clear_list(&h->a);
	clear_list(&h->b);
	h->size = 0;
	h->capacity = 0;
	return 1;
}

uint64_t hash1(const char *str) {
	uint64_t hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

uint64_t hash2(const char *str) {
	uint64_t hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

void next_iter_hashtable(Iter *self) {
	*(int*) self->data[1] += 1;
}

int done_iter_hashtable(const Iter *self) {
	return *(int*)self->data[1] < *(int*)self->data[2];
}

void *val_iter_hashtable(const Iter *self) {
	return access_list(self->data[0], *(int*) self->data[1]);
}

int iter_hashtable(Iter *self, Hashtable *h) {
	self->data = calloc(sizeof(void*), 3);
	self->data[0] = malloc(sizeof(List));
	ctor_list(self->data[0]);

	Iter iter1, iter2;
	iter_list(&iter1, &h->a);
	iter_list(&iter2, &h->b);
	for (size_t i = 0; i < h->a.length; i++) {
		if (iter1.val(&iter1)) {
			append_list(self->data[0], iter1.val(&iter1));
		}
		if (iter2.val(&iter2)) {
			append_list(self->data[0], iter2.val(&iter2));
		}
		iter1.next(&iter1);
		iter2.next(&iter2);
	}
	destroy_iter_list(&iter1);
	destroy_iter_list(&iter2);

	self->data[1] = malloc(sizeof(int));
	self->data[2] = malloc(sizeof(int));
	*(int*)self->data[1] = 0;
	*(int*)self->data[2] = h->size;
	self->next = &next_iter_hashtable;
	self->done = &done_iter_hashtable;
	self->val = &val_iter_hashtable;
	return 1;
}

void destroy_iter_hashtable(Iter *self) {
	dtor_list(self->data[0]);
	free(self->data[0]);
	free(self->data[1]);
	free(self->data[2]);
	free(self->data);
}
