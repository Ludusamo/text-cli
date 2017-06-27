#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

size_t aggregate_data_to_string(char *ptr, size_t size, size_t nmemb, char **s) {
	size_t old_len = *s ? strlen(*s) : 0;
	size_t new_len = old_len + size * nmemb;
	*s = realloc(*s, new_len + 1);
	memcpy(*s + old_len, ptr, size * nmemb);
	(*s)[new_len] = '\0';

	return size * nmemb;
}

const char *get_texts() {
	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	char *s = 0;
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL,
		                 "https://script.google.com/macros/s/AKfycbzI4xNm7OuDXkB-O6srZvBB4C0pR3J2uDS3wVHuH15VCbOR-ZQ/exec?intent=all&sheet=received");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, aggregate_data_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			        curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return s;
}

int main() {
	printf("%s\n", get_texts());
	return 0;
}
