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
using namespace restbed;

void get_hotwords(const string key, string &body){
	Redis *r = new Redis();
	if(!r->connect("127.0.0.1",6379)){
		fprintf(stdout,"Connect Redis failed");
		return;
	}

	vector<string> items;
	r->zrevrangebyscore(key,items);
	
	for(int i = 0; i < items.size(); i++){
		body += items[i];
		if(i<int(items.size())-1) body += ",";
	}
	//fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
}

void get_method_handler(const shared_ptr<Session> session){
	const auto request = session->get_request();
	const string hot_date = request->get_query_parameter("hot");
	const string record_date = request->get_query_parameter("record");
	const string search_info = request->get_query_parameter("search");
	string body;
	if(!hot_date.empty()){
		get_hotwords(hot_date, body);
	} else if (!record_date.empty()) {
		select_one_day_from_mysql(record_date, body);	
	} else if (!search_info.empty()) {
		search_in_es(search_info, body);	
	}
	session->close(OK, body, {{"Content-Length", ::to_string(body.size())}});
}

void write_keywords_to_redis(const rapidjson::Document &json_doc) {
  string show_status = "Write to Redis.";
	fprintf(stdout, "%.*s\n", (int)show_status.size(), show_status.c_str());
	
  Redis *r = new Redis();
  if(!r->connect("127.0.0.1", 6379)){
      fprintf(stdout, "%.*s\n", 20, "Connect Redis failed");
      return;
  }
 
  vector<string> words;
  string keywords, time_str;
  
  if(json_doc.HasMember("keywords")) {
      keywords = json_doc["keywords"].GetString();
  }
  if(json_doc.HasMember("time")) {
      time_str = json_doc["time"].GetString();
  }
  string temp_word;
  for(char c:keywords) {
      if(c == ',') {
          words.push_back(temp_word);
          temp_word.clear();
      }
      else temp_word += c;
  }
  if(temp_word.size()) words.push_back(temp_word);
  if(words.size() && time_str.size()) {
      r->zadd_words(time_str, words);
      delete r;
      r=NULL;
  }
}

void write_record_to_mysql(const rapidjson::Document &json_doc) {
  string show_status = "Write to MySQL.";
	fprintf(stdout, "%.*s\n", (int)show_status.size(), show_status.c_str());

  string cmd;
  stringstream scmd;
  scmd << "INSERT INTO resource (title, author, keywords, time, content) VALUES (\"";
  scmd << json_doc["title"].GetString() << "\", \"" <<  \
         json_doc["author"].GetString() << "\", \"" << \
         json_doc["keywords"].GetString() << "\", \"" << \
         json_doc["time"].GetString() << "\", \"" << \
         json_doc["content"].GetString() << "\");";
  cmd = scmd.str();
	//fprintf(stdout, "%.*s\n", (int)cmd.size(), cmd.c_str());
  insert_record_to_mysql(cmd);
}

void post_method_handler(const shared_ptr<Session> session){
	const auto request = session->get_request();
	size_t content_size = request->get_header("Content-Length", 0);
	
	auto handle_body = [request](const shared_ptr<Session> session,const Bytes &body){	
		//fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
		
    rapidjson::Document json_doc;
    //Convert unsighed char* to char*
    char temp[body.size() + 1];
    memcpy(temp, body.data(), body.size());
    temp[body.size()] = '\0';
    json_doc.Parse(temp);

    write_keywords_to_redis(json_doc);
    write_record_to_mysql(json_doc);
		session->close(OK, "Post complete!", {{"Content-Length", "14"}, {"Connection", "close"}});
	};

	session->fetch(content_size, handle_body);	
}


int main(void){
	cout<<"Start restbed."<<endl;
  create_table_in_mysql();

	auto resource = make_shared<Resource>();
	resource->set_path("/resource");
	resource->set_method_handler("POST", post_method_handler);
	resource->set_method_handler("GET", get_method_handler);

	auto settings = make_shared<Settings>();
	settings->set_port(1984);
	settings->set_default_header("Connection", "close");

	Service service;
	service.publish(resource);
	service.start(settings);

	return EXIT_SUCCESS;
}
