#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include "obj.h"

typedef union {
	double as_double;
	uint64_t bits;
	int32_t as_int32;
} Value;

static const Value nil_val = { .bits = 0xFFF00000ULL << 32 };
static const Value true_val = { .bits = 0xFFF00001ULL << 32 };
static const Value false_val = { .bits = 0xFFF00002ULL << 32 };
static const uint64_t max_double = 0xFFF8ULL << 48;
static const uint64_t ptr_mask = 0xFFF9ULL << 48;
static const uint64_t int_mask = 0xFFFAULL << 48;
static const uint64_t obj_mask = 0xFFFBULL << 48;

int is_neg_zero(double d);
int is_double(Value v);
int is_ptr(Value v);
int is_int32(Value v);
int is_obj(Value v);

Value from_double(double d);
Value from_ptr(void *p);
Value from_obj(Obj *obj);

void *get_ptr(Value v);
Obj *get_obj(Value v);

#endif // VALUE_H
