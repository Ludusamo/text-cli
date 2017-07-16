#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "json.h"

size_t aggregate_data_to_string(char *ptr, size_t size, size_t nmemb, char **s) {
	size_t old_len = *s ? strlen(*s) : 0;
	size_t new_len = old_len + size * nmemb;
	*s = realloc(*s, new_len + 1);
	memcpy(*s + old_len, ptr, size * nmemb);
	(*s)[new_len] = '\0';

	return size * nmemb;
}

char *get_texts() {
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	char *s = 0;
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL,
		                 "https://script.google.com/macros/s/AKfycbzI4xNm7OuDXkB-O6srZvBB4C0pR3J2uDS3wVHuH15VCbOR-ZQ/exec?intent=retrieve&sheet=received&numRows=10");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, aggregate_data_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			        curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl = 0;
	}
	return s;
}

int main() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	char *s = get_texts();
	//printf("%s\n", s);
	curl_global_cleanup();

	printf("Text CLI\n");
	List l;
	ctor_list(&l);
	Json *json = parse_json(s);
	char *jstr = json_stringify(json);
	//printf("%s\n", jstr);
	Iter i;
	iter_hashtable(&i, &json->h);
	foreach(i) {
		printf("%lx - %s : %lx\n",
				i.val(&i).bits,
		       ((Keyval*) get_ptr(i.val(&i)))->key,
		       ((Keyval*) get_ptr(i.val(&i)))->val.bits);
	}
	destroy_iter_hashtable(&i);
	printf("Is obj? %d\n", is_type_json(json, "data", jOBJ));
	printf("Is arr? %d\n", is_type_json(json, "data", jLIST));
	List *texts = retrieve_list_json(json, "data");
	for (size_t i = 0; i < texts->length; i++) {
		List *text = get_obj(access_list(texts, i))->ptr;
		printf("%lx\n", (access_list(text, 0)).bits);
		printf("Num: %s, Msg: %s, Timestamp: %d\n",
		       get_ptr(access_list(text, 0)),
		       get_ptr(access_list(text, 1)),
	           (access_list(text, 2)).as_int32);
	}
	free(jstr);
	free(s);
	destroy_json(json);
	return 0;
}
