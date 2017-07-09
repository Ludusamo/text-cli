#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <string.h>
#include "iter.h"
#include "value.h"

typedef struct {
	Value *data;
	size_t length;
} Array;

int ctor_array(Array *array, size_t len);
int dtor_array(Array *array);

int set_array(Array *array, int index, Value val);
Value access_array(const Array *array, int index);
Array *add_array(Array *arr1, Array *arr2);

void next_iter_array(Iter *self);
int done_iter_array(const Iter *self);
Value val_iter_array(const Iter *self);
int iter_array(Iter *iter, Array *array);
void destroy_iter_array(Iter *iter);

#endif // ARRAY_H
