#ifndef ITER_H
#define ITER_H

#include "value.h"

#define foreach(iter) for(; iter.done(&iter); iter.next(&iter))

typedef struct Iter Iter;

struct Iter {
	void **data;
	void (*next)(Iter *self);
	int (*done)(const Iter *self);
	Value (*val)(const Iter *self);
};

#endif // ITER_H
