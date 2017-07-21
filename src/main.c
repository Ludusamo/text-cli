#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "convo_select.h"
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
		                 "https://script.google.com/macros/s/AKfycbzI4xNm7OuDXkB-O6srZvBB4C0pR3J2uDS3wVHuH15VCbOR-ZQ/exec?intent=retrieve&sheet=received&numRows=200");
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
	free(s);
	
	List *texts = retrieve_list_json(json, "data");
	Hashtable convos;
	ctor_hashtable(&convos);
	for (size_t i = 0; i < texts->length; i++) {
		List *text = get_obj(access_list(texts, i))->ptr;
		const char *num = get_ptr(access_list(text, 0));
		Value cur_num = access_hashtable(&convos, num);
		if (cur_num.bits == nil_val.bits) cur_num = from_double(0);
		set_hashtable(&convos, num, from_double(++cur_num.as_int32));
	}

	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);

	Convo_Select cs;
	init_convo_select(&cs);

	Iter i;
	iter_hashtable(&i, &convos);
	foreach (i) {
		Keyval *convo = get_ptr(i.val(&i));
		add_convo(&cs, convo->key, "0");
	}

	destroy_iter_hashtable(&i);
	render_convo_select(&cs);
	update_convo_select(&cs);
	deinit_convo_select(&cs);

	dtor_hashtable(&convos);
	destroy_json(json);
	endwin();
	return 0;
}
