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
		vector<string> check_list{"title", "author", "keywords", "time", "content"};
		bool valid_input = true;
		for(string key : check_list) {
			if(json_doc.HasMember(key.c_str())) {
				string value = json_doc[key.c_str()].GetString();
				if(value.empty()){
					valid_input = false;
					break;
				}
			}
			else {
				valid_input = false;
				break;
			}
		}
		
		int write_redis_flag;
		int write_mysql_flag;
		if(valid_input) {
			string show_status = "Write Record.";
			fprintf(stdout, "%.*s\n", (int)show_status.size(), show_status.c_str());
			write_redis_flag = WriteKeywordsToRedis(json_doc);
			write_mysql_flag = WriteRecordToMysql(json_doc);
		}
		
		string write_res;
		if(write_redis_flag == 0 && write_mysql_flag == 0) 
			write_res = "Post success!";
		else write_res = "Post fail!";
		string content_len = to_string(write_res.size());
    session->close(restbed::OK, write_res.c_str(), {{"Content-Length", \
			content_len.c_str()}, {"Connection", "close"}});
  };

  session->fetch(content_size, handle_body);  
}

void GetSearchHandler(const shared_ptr<restbed::Session> session){
  const auto request = session->get_request();
  const string keywords = request->get_query_parameter("keywords");
  const string limit = request->get_query_parameter("limit");
  const string offset = request->get_query_parameter("offset");
  
  string search_res;
  if(!keywords.empty()){	
    SearchInES(keywords, limit, offset, search_res);
	}
  session->close(restbed::OK, search_res, {{"Content-Length", ::to_string(search_res.size())}});
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
  resource_search->set_method_handler("GET", GetSearchHandler);

  auto settings = make_shared<restbed::Settings>();
  settings->set_port(1984);
  settings->set_default_header("Connection", "close");

  restbed::Service service;
  service.publish(resource);
  service.publish(resource_search);
  service.start(settings);

  return EXIT_SUCCESS;
}
