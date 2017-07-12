#include "array.h"

int ctor_array(Array *array, size_t len) {
	array->data = calloc(sizeof(Value), len);
	array->length = len;
	return 1;
}

int dtor_array(Array *array) {
	if (array) {
		if (array->data) {
			free(array->data);
			array->data = 0;
		}
		array->length = 0;
	}
	return 1;
}

int set_array(Array *array, int index, Value val) {
	if (!array || !(array->data) || index >= array->length) return 0;
	array->data[index] = val;
	return 1;
}

Value access_array(const Array *array, int index) {
	if (!array || !(array->data) || index >= array->length) return nil_val;
	return array->data[index];
}

Array *add_array(Array *arr1, Array *arr2) {
	Array *new_array = malloc(sizeof(Array));
	ctor_array(new_array, arr1->length + arr2->length);
	int index = 0;
	Iter i;
	iter_array(&i, arr1);
	foreach(i) {
		set_array(new_array, index++, i.val(&i));
	}
	destroy_iter_array(&i);
	iter_array(&i, arr2);
	foreach(i) {
		set_array(new_array, index++, i.val(&i));
	}
	destroy_iter_array(&i);
	return new_array;
}

void next_iter_array(Iter *self) {
	*((int*)self->data[1]) += 1;
}

int done_iter_array(const Iter *self) {
	return *((int*)self->data[1]) < *((int*) self->data[2]);
}

Value val_iter_array(const Iter *self) {
	return access_array((Array*) self->data[0], *((int*) self->data[1]));
}

int iter_array(Iter *iter, const Array *array) {
	iter->data = calloc(sizeof(Value), 3);
	iter->data[0] = (Array *) array;
	iter->data[1] = malloc(sizeof(int));
	iter->data[2] = malloc(sizeof(int));
	*((int*)(iter->data[1])) = 0;
	*((int*)(iter->data[2])) = array->length;
	iter->next = &next_iter_array;
	iter->done = &done_iter_array;
	iter->val = &val_iter_array;
	return 1;
}

void destroy_iter_array(Iter *iter) {
	free(iter->data[1]);
	free(iter->data[2]);
	free(iter->data);
}
