#include <stdio.h>
#include "json.h"

int main() {
	printf("Text CLI\n");
	List l;
	ctor_list(&l);
	Json *json = parse_json("{\"test\": [\"hello\", 57], \"test2\": 34}");
	const char *jstr = json_stringify(json);
	printf("%s\n", jstr);
	return 0;
}
