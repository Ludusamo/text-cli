#ifndef LIST_H
#define LIST_H

#include "array.h"

typedef struct {
	Array array;
	size_t length;
} List;

int ctor_list(List *list);
int dtor_list(List *list);

int insert_list(List *list, int index, Value val);
int delete_list(List *list, int index);
int append_list(List *list, Value val);
Value pop_list(List *list);
int clear_list(List *list);
int set_list(List *list, int index, Value val);
Value access_list(const List *list, int index);
List *add_list(List *lhs, List *rhs);
int resize_list(List *list, size_t len, Value value);
int reserve_space_list(List *list, size_t len);

int _grow_list(List *list);

void next_iter_list(Iter *self);
int done_iter_list(const Iter *self);
Value val_iter_list(const Iter *self);

int iter_list(Iter *self, const List *list);
void destroy_iter_list(Iter *self);

#endif // LIST_H
