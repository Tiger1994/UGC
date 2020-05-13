#include"rapidjson/document.h"
#include<vector>
#include<ctime>
#include<string>
#include<restbed>
#include<cstdlib>
#include<iostream>
#include <sstream>
#include"myredis/myredis.h"
#include "my_mysql/my_mysql.h"
#include "mycurl/mycurl.h"

using namespace std;

void get_method_handler(const shared_ptr<restbed::Session> session){
	const auto request = session->get_request();
	const string hot_date = request->get_query_parameter("hot");
	const string record_date = request->get_query_parameter("record");
	const string search_info = request->get_query_parameter("search");
	const string limit = request->get_query_parameter("limit");
	const string offset = request->get_query_parameter("offset");
	
	string body;
	if(!hot_date.empty()){
		get_hotwords(hot_date, body);
	} else if (!record_date.empty()) {
		select_one_day_from_mysql(record_date, limit, offset, body);	
	} else if (!search_info.empty()) {
		search_in_es(search_info, limit, offset, body);	
	}
	session->close(restbed::OK, body, {{"Content-Length", ::to_string(body.size())}});
}

void post_method_handler(const shared_ptr<restbed::Session> session){
	const auto request = session->get_request();
	size_t content_size = request->get_header("Content-Length", 0);
	
	auto handle_body = [request](const shared_ptr<restbed::Session> session,const restbed::Bytes &body){	
		//fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
    rapidjson::Document json_doc;
    //Convert unsighed char* to char*
    char temp[body.size() + 1];
    memcpy(temp, body.data(), body.size());
    temp[body.size()] = '\0';
    json_doc.Parse(temp);

    write_keywords_to_redis(json_doc);
    write_record_to_mysql(json_doc);
		session->close(restbed::OK, "Post complete!", {{"Content-Length", "14"}, {"Connection", "close"}});
	};

	session->fetch(content_size, handle_body);	
}


int main(void){
	cout<<"Start restbed."<<endl;
  create_table_in_mysql();

	auto resource = make_shared<restbed::Resource>();
	resource->set_path("/resource");
	resource->set_method_handler("POST", post_method_handler);
	resource->set_method_handler("GET", get_method_handler);

	auto settings = make_shared<restbed::Settings>();
	settings->set_port(1984);
	settings->set_default_header("Connection", "close");

	restbed::Service service;
	service.publish(resource);
	service.start(settings);

	return EXIT_SUCCESS;
}
