#include"rapidjson/document.h"
#include<vector>
#include<ctime>
#include<string>
#include<restbed>
#include<cstdlib>
#include<iostream>
#include"myredis/redis.h"

using namespace std;
using namespace restbed;
using namespace rapidjson;

void get_hotwords(const string key, string &body){
	Redis *r = new Redis();
	if(!r->connect("127.0.0.1",6379)){
		fprintf(stdout,"Connect Redis failed");
		return;
	}

	//int t=time(0)/86400;//In Unix time, 1 day counts 86400.
	//string t_str=to_string(t);

	vector<string> items;
	r->zrevrangebyscore(key,items);
	
	for(int i = 0; i < items.size(); i++){
		body += items[i];
		if(i<int(items.size())-1) body += " ";
	}
	fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
}

void get_method_handler(const shared_ptr<Session> session){
	const auto request = session->get_request();
	const string hot_date = request->get_query_parameter("hot");
	string body;
	if(!hot_date.empty()){
		fprintf(stdout,"%.*s\n",20,"Getting Hot words.");
		get_hotwords(hot_date, body);
	}
	session->close(OK, body, {{"Content-Length", ::to_string(body.size())}});
}


void post_method_handler(const shared_ptr<Session> session){
	const auto request = session->get_request();
	size_t content_size = request->get_header("Content-Length", 0);
	
	auto handle_body = [request](const shared_ptr<Session> session,const Bytes &body){	
		fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
		
		Redis *r = new Redis();
		if(!r->connect("127.0.0.1", 6379)){
			fprintf(stdout, "Connect Redis failed");
			return;
		}
		
		int t = time(0)/86400;  //In Unix time, 1 day counts 86400.
		string t_str = to_string(t);
		Document doc;

		//Convert unsighed char* to char*
		char temp[body.size()+1];
		memcpy(temp,body.data(), body.size()+1);
		string s_temp(temp);

		doc.Parse(s_temp.c_str());
		vector<string> words;
		for(size_t i = 0; i<doc["keywords"].Size(); i++){
			words.push_back(doc["keywords"][i].GetString());
		}
		r->zadd_words(t_str, words);
		delete r;
		r=NULL;

		session->close(OK, "Post complete!", {{"Content-Length", "14"}, {"Connection", "close"}});
	};

	session->fetch(content_size, handle_body);	
}


int main(void){
	cout<<"Start restbed."<<endl;

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
