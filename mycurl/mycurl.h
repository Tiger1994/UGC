#ifndef _MYCURL_MYCURL_H_
#define _MYCURL_MYCURL_H_

#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>

#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/curlbuild.h>

#include "../util/urlcode.h"

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  std::string data((const char*) ptr, (size_t) size * nmemb);
  *((std::stringstream*) stream) << data;
  return size*nmemb;
}

int SearchInES(const std::string &search_info, const std::string str_limit, \
    const std::string str_offset, std::string &search_res) {
  int limit = 10, offset = 0;
  if(!str_limit.empty()) limit = std::stoi(str_limit);
  if(!str_offset.empty()) offset = std::stoi(str_offset);

  std::string search_info_decode = UrlDecode(search_info);
  rapidjson::StringBuffer str_buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
  writer.StartObject();
  writer.Key("query");
  writer.StartObject();
  writer.Key("match");
  writer.StartObject();
  writer.Key("content");

  writer.String(search_info_decode.c_str());
  writer.Key("analyzer");
  writer.String("ik_max_word");
  writer.EndObject(); 
  writer.EndObject(); 
  writer.EndObject(); 

  std::stringstream out;
  std::string query = str_buf.GetString();

  fprintf(stdout, "%.*s\n", int(query.size()), query.c_str());
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

  //fprintf(stdout, "%.*s\n", int(out_s.size()), out_s.c_str());
  doc.Parse(out_s.c_str());
  if(doc.HasMember("error")) {
    return 0;
  } 
  rapidjson::Value& hits = doc["hits"]["hits"];
  int total = doc["hits"]["total"].GetInt();
  rapidjson::StringBuffer str_buf2;
  rapidjson::Writer<rapidjson::StringBuffer> writer2(str_buf2);
  writer2.StartObject();
  writer2.Key("total");
  writer2.Int(total);
  std::string num_find = std::to_string(doc["hits"]["total"].GetInt());
  num_find = "Searched " + num_find + " record(s).";
  fprintf(stdout, "%.*s\n", int(num_find.size()), num_find.c_str());
  size_t ceil = offset + limit > total ? total : offset + limit;
  for (size_t i = offset; i < ceil; i++) {
    std::string str_index = std::to_string(i);
    writer2.Key(str_index.c_str());
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
