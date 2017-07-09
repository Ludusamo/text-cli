#include <stdio.h>
#include "json.h"

int main() {
	printf("Text CLI\n");
	List l;
	ctor_list(&l);
	_tokenize_json(&l, "{\n},true : -34 false null\"hello\" 1.3 -354.5(\n)");
	Iter i;
	iter_list(&i, &l);
	foreach(i) {
		print_token(get_ptr(i.val(&i)));
	}
	return 0;
}
