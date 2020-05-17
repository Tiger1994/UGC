#include <vector>
#include <ctime>
#include <string>
#include <restbed>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "myredis/myredis.h"
#include "my_mysql/my_mysql.h"
#include "rapidjson/document.h"
#include "mycurl/mycurl.h"

using namespace std;

void GetMethodHandler(const shared_ptr<restbed::Session> session){
  const auto request = session->get_request();
  const string hot_date = request->get_query_parameter("hot");
  const string record_date = request->get_query_parameter("record");
  const string limit = request->get_query_parameter("limit");
  const string offset = request->get_query_parameter("offset");
  
  string body;
  if(!hot_date.empty()){
    GetHotwords(hot_date, body);
  } else if (!record_date.empty()) {
    SelectOneDayFromMysql(record_date, limit, offset, body);  
  }
  session->close(restbed::OK, body, {{"Content-Length", ::to_string(body.size())}});
}

void PostRecordHandler(const shared_ptr<restbed::Session> session){
  const auto request = session->get_request();
  size_t content_size = request->get_header("Content-Length", 0);
  
  auto handle_body = [request](const shared_ptr<restbed::Session> session, \
			const restbed::Bytes &body){  
    //fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
    rapidjson::Document json_doc;
    //Convert unsighed char* to char*
    char temp[body.size() + 1];
    memcpy(temp, body.data(), body.size());
    temp[body.size()] = '\0';
    json_doc.Parse(temp);
		bool valid_title = false;
		bool valid_author = false;
		bool valid_keywords = false;
		bool valid_time = false;
		bool valid_content = false;
		
		if(json_doc.HasMember("title")) {
			string title = json_doc["title"].GetString();
			if(!title.empty()) valid_title = true;
		}	
 		if(json_doc.HasMember("author")) {
			string title = json_doc["author"].GetString();
			if(!title.empty()) valid_author = true;
		}	
 		if(json_doc.HasMember("keywords")) {
			string title = json_doc["keywords"].GetString();
			if(!title.empty()) valid_keywords = true;
		}	
 		if(json_doc.HasMember("time")) {
			string title = json_doc["time"].GetString();
			if(!title.empty()) valid_time = true;
		}	
 		if(json_doc.HasMember("content")) {
			string title = json_doc["content"].GetString();
			if(!title.empty()) valid_content = true;
		}
		if(valid_title && valid_author && valid_keywords && valid_time && valid_content) {
			WriteKeywordsToRedis(json_doc);
			WriteRecordToMysql(json_doc);
		}
    session->close(restbed::OK, "Post complete!", {{"Content-Length", "14"}, \
			{"Connection", "close"}});
  };

  session->fetch(content_size, handle_body);  
}

void PostSearchHandler(const shared_ptr<restbed::Session> session){
  const auto request = session->get_request();
  size_t content_size = request->get_header("Content-Length", 0);
  
  auto handle_body = [request](const shared_ptr<restbed::Session> session, \
			const restbed::Bytes &body){  
    //fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
    const string limit = request->get_query_parameter("limit");
    const string offset = request->get_query_parameter("offset");
    //Convert unsighed char* to char*
    char temp[body.size() + 1];
    memcpy(temp, body.data(), body.size());
    temp[body.size()] = '\0';
    string search_info(temp);
    string search_res;
    SearchInES(search_info, limit, offset, search_res);
    string res_len = to_string(int(search_res.size()));
    session->close(restbed::OK, search_res.c_str(), {{"Content-Length", res_len}});
  };

  session->fetch(content_size, handle_body);  
}

int main(void){
  cout << "Start restbed." << endl;
  CreateTableInMysql();

  auto resource = make_shared<restbed::Resource>();
  resource->set_path("/resource");
  resource->set_method_handler("POST", PostRecordHandler);
  resource->set_method_handler("GET", GetMethodHandler);

  auto resource_search = make_shared<restbed::Resource>();
  resource_search->set_path("/resource/_search");
  resource_search->set_method_handler("POST", PostSearchHandler);

  auto settings = make_shared<restbed::Settings>();
  settings->set_port(1984);
  settings->set_default_header("Connection", "close");

  restbed::Service service;
  service.publish(resource);
  service.publish(resource_search);
  service.start(settings);

  return EXIT_SUCCESS;
}
