#ifndef _MYCURL_MYCURL_H_
#define _MYCURL_MYCURL_H_

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/curlbuild.h>
#include <sstream>
#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <unistd.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	string data((const char*) ptr, (size_t) size * nmemb);
	*((stringstream*) stream) << data;
  return size*nmemb;
}

int search_in_es(const std::string &search_info, string &search_res) {
	std::stringstream out;
	void* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/resource/_search");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, search_info.c_str());
	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
  search_res = out.str();
  return 0;
}
#endif
