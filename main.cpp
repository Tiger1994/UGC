#include"rapidjson/document.h"
#include<vector>
#include<ctime>
#include<string>
#include<restbed>
#include<cstdlib>
#include<iostream>
#include"myredis/myredis.h"

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

void write_keywords_to_redis(Document &json_doc) {	
	Redis *r = new Redis();
    if(!r->connect("127.0.0.1", 6379)){
        fprintf(stdout, "Connect Redis failed");
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

void post_method_handler(const shared_ptr<Session> session){
	const auto request = session->get_request();
	size_t content_size = request->get_header("Content-Length", 0);
	
	auto handle_body = [request](const shared_ptr<Session> session,const Bytes &body){	
		fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
		
        Document json_doc;
        //Convert unsighed char* to char*
        char temp[body.size() + 1];
        memcpy(temp,body.data(), body.size() + 1);
        string s_temp(temp);
        json_doc.Parse(s_temp.c_str());
        
        write_keywords_to_redis(json_doc);
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
