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
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <vector>

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	std::string data((const char*) ptr, (size_t) size * nmemb);
	*((std::stringstream*) stream) << data;
  return size*nmemb;
}

int search_in_es(const std::string &search_info, std::string &search_res) {
	rapidjson::StringBuffer str_buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
	writer.StartObject();
	writer.Key("query");
	writer.StartObject();
	writer.Key("match");
	writer.StartObject();
	writer.Key("content");

	// convert comma to space.
	std::string temp;
	for(char c:search_info) {
		 if(c == ',') {
				temp += ' ';
		 }
		 else temp += c;
	}
	writer.String(temp.c_str());
	writer.EndObject();	
	writer.EndObject();	
	writer.EndObject();	

  std::stringstream out;
	std::string query = str_buf.GetString();
	void* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/resource/_search?pretty");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
	
	rapidjson::Document doc;
	std::string out_s = out.str();
	doc.Parse(out_s.c_str());
	rapidjson::Value& hits = doc["hits"]["hits"];
  
	rapidjson::StringBuffer str_buf2;
	rapidjson::Writer<rapidjson::StringBuffer> writer2(str_buf2);
	writer2.StartObject();

	std::string num_find = std::to_string(doc["hits"]["total"].GetInt());
	num_find = "Searched " + num_find + " record(s).";
	fprintf(stdout, "%.*s\n", num_find.size(), num_find.c_str());
  for (size_t i = 0; i < doc["hits"]["total"].GetInt(); i++) {
    writer2.Key(std::to_string(hits[i]["_source"]["id"].GetInt()).c_str());
    writer2.StartObject();
    writer2.Key("title");
    writer2.String(hits[i]["_source"]["title"].GetString());
    writer2.Key("author");
    writer2.String(hits[i]["_source"]["author"].GetString());
    writer2.Key("keywords");
    writer2.String(hits[i]["_source"]["keywords"].GetString());
    writer2.Key("time");
    writer2.String(hits[i]["_source"]["time"].GetString());
    writer2.Key("content");
    writer2.String(hits[i]["_source"]["content"].GetString());
    writer2.EndObject();
  }
	writer2.EndObject();
  search_res = str_buf2.GetString();
  return 0;
}
#endif