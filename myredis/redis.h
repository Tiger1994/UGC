#ifndef _REDIS_H_
#define _REDIS_H_

#include<iostream>
#include<string.h>
#include<string>
#include<stdio.h>
#include<vector>
#include<hiredis/hiredis.h>

class Redis{
public:
	Redis(){}

	~Redis(){
		this->_connect=NULL;
		this->_reply=NULL;
	}
	
	bool connect(std::string host,int port){
		this->_connect=redisConnect(host.c_str(),port);
		if(this->_connect!=NULL&&this->_connect->err){
			printf("connection error: %s\n",this->_connect->errstr);
			return 0;
		}
		return 1;
	}
	
	std::string get(std::string key){
		this->_reply=(redisReply*)redisCommand(this->_connect,"GET %s",key.c_str());
		std::string str=this->_reply->str;
		freeReplyObject(this->_reply);
		return str;
	}

	int zadd(std::string key,int score,std::string value){
		std::string cmd="ZADD "+key+" "+std::to_string(score)+" "+value;
		this->_reply=(redisReply*)redisCommand(this->_connect,cmd.c_str());
		int res=this->_reply->integer;
		/*if(res){
	   		fprintf(stdout,"ADD Redis success!");
		}
		else fprintf(stdout,"ADD Redis failed!");*/
		
		//fprintf(stdout,"%.*s\n",20,value.c_str());
		freeReplyObject(this->_reply);
		return res;
	}

	void zadd_words(std::string key,std::vector<std::string> words){
		for(std::string w:words){
			std::string cmd="ZINCRBY "+key+" 1 "+w;
			this->_reply=(redisReply*)redisCommand(this->_connect,cmd.c_str());
			freeReplyObject(this->_reply);
		}
	}

	int zrangebyscore(std::string key,std::vector<std::string> &items){
		std::string cmd="ZRANGEBYSCORE "+key+" -inf 5 limit 0 100";
		this->_reply=(redisReply*)redisCommand(this->_connect,cmd.c_str());
		int res=0;
		if(this->_reply&&this->_reply->type==REDIS_REPLY_ARRAY){
			std::string str=std::to_string(this->_reply->elements);
			fprintf(stdout,"%.*s\n",20,str.c_str());
			for(size_t i=0;i<this->_reply->elements;i++){
				redisReply *ele=this->_reply->element[i];
				if(ele->type==REDIS_REPLY_INTEGER){
					items.push_back(std::to_string(ele->integer));
				}
				else if(ele->type==REDIS_REPLY_STRING){
					std::string s(ele->str,ele->len);
					items.push_back(s);
				}
			}
			res=this->_reply->elements;
		}
		else fprintf(stdout,"%.*s\n",20,"No Hot Words.");
		freeReplyObject(this->_reply);
		return res;
	}

	void set(std::string key,std::string value){
		redisCommand(this->_connect,"SET %s %s",key.c_str(),value.c_str());
	}
private:
	redisContext* _connect;
	redisReply* _reply;
};

#endif
