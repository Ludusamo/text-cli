#include <stdio.h>
#include "json.h"

int main() {
	printf("Text CLI\n");
	List l;
	ctor_list(&l);
	Json *json = parse_json("{\"test\": [\"hello\", 57], \"test2\": 34}");
	char *jstr = json_stringify(json);
	printf("%s\n", jstr);
	free(jstr);
	destroy_json(json);
	return 0;
}
