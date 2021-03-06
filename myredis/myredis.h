#ifndef UGC_MYREDIS_MYREDIS_H_
#define UGC_MYREDIS_MYREDIS_H_

#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>

#include <hiredis/hiredis.h>
#include <rapidjson/document.h>

class Redis{
public:
  Redis(){}

  ~Redis(){
    this->connetc_ = NULL;
    this->reply_ = NULL;
  }
  
  bool connect(std::string host,int port){
    this->connetc_ = redisConnect(host.c_str(), port);
    if(this->connetc_ != NULL && this->connetc_->err){
      printf("connection error: %s\n", this->connetc_->errstr);
      return 0;
    }
    return 1;
  }
  
  std::string get(std::string key){
    this->reply_ = (redisReply*)redisCommand(this->connetc_, "GET %s", key.c_str());
    std::string str = this->reply_->str;
    freeReplyObject(this->reply_);
    return str;
  }

  int zadd(std::string key, int score, std::string value){
    std::string cmd = "ZADD " + key + " " + std::to_string(score) + " " + value;
    this->reply_ = (redisReply*)redisCommand(this->connetc_, cmd.c_str());
    int res = this->reply_->integer;
    freeReplyObject(this->reply_);
    return res;
  }

  int zadd_words(std::string key, std::vector<std::string> words){
    for(std::string w:words){
      std::string cmd = "ZINCRBY " + key + " 1 " + "\"" + w + "\"";
      this->reply_ = (redisReply*)redisCommand(this->connetc_,cmd.c_str());
      if(this->reply_->type == REDIS_REPLY_ERROR) return 1;
      // fprintf(stdout, "%.*s\n", int(cmd.size()), cmd.c_str());
			freeReplyObject(this->reply_);
    }
    return 0;
  }

  int zrevrangebyscore(std::string key, std::vector<std::string> &items){
    std::string cmd = "ZREVRANGEBYSCORE " + key + " inf 1 limit 0 100";
    this->reply_ = (redisReply*)redisCommand(this->connetc_, cmd.c_str());
    int res = 0;
    if(this->reply_ && this->reply_->type == REDIS_REPLY_ARRAY){
      std::string str = std::to_string(this->reply_->elements);
      str = "Get " + str + " Hotwords";
      fprintf(stdout, "%.*s\n", int(str.size()), str.c_str());
      
      for(size_t i = 0; i < this->reply_->elements; i++){
        redisReply *ele = this->reply_->element[i];
        if(ele->type == REDIS_REPLY_INTEGER) {
          items.push_back(std::to_string(ele->integer));
        } else if(ele->type==REDIS_REPLY_STRING) {
          std::string s(ele->str, ele->len);
          items.push_back(s);
        }
      }
      res = this->reply_->elements;
    }
    else fprintf(stdout, "%.*s\n", 20, "No Hot Words.");
    freeReplyObject(this->reply_);
    return res;
  }

  void set(std::string key, std::string value){
    redisCommand(this->connetc_, "SET %s %s", key.c_str(), value.c_str());
  }
private:
  redisContext* connetc_;
  redisReply* reply_;
};

void GetHotwords(const std::string key, std::string &body){
  Redis *r = new Redis();
  if(!r->connect("127.0.0.1",6379)){
    fprintf(stdout,"Connect Redis failed");
    return;
  }

  std::vector<std::string> items;
  r->zrevrangebyscore(key,items);

  for(int i = 0; i < int(items.size()); i++){
    body += items[i];
    if(i < int(items.size())-1) body += ",";
  }
  //fprintf(stdout, "%.*s\n", (int)body.size(),body.data());
}

int WriteKeywordsToRedis(const rapidjson::Document &json_doc) {
  // std::string show_status = "Write to Redis.";
  // fprintf(stdout, "%.*s\n", (int)show_status.size(), show_status.c_str());
  Redis *r = new Redis();
  if(!r->connect("127.0.0.1", 6379)){
      fprintf(stdout, "%.*s\n", 20, "Connect Redis failed");
      return 1;
  }   
  
  std::string keywords, time_str;
  if(json_doc.HasMember("keywords")) {
      keywords = json_doc["keywords"].GetString();
  }
  if(json_doc.HasMember("time")) {
      time_str = json_doc["time"].GetString();
  }
  //fprintf(stdout, "%.*s\n", int(keywords.size()), keywords.c_str());
  std::vector<std::string> words;
  std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  std::wstring temp_word;
  std::wstring w_keywords = conv.from_bytes(keywords);
  std::locale loc("en_US.UTF-8");
  for(wchar_t wc:w_keywords) {
      if(std::ispunct(wc, loc)) {
          words.push_back(conv.to_bytes(temp_word));
          temp_word.clear();
      }
      else temp_word += wc;
  }
  if(temp_word.size()) words.push_back(conv.to_bytes(temp_word));

  int res = 0;
  if(words.size() && time_str.size()) {
      res = r->zadd_words(time_str, words);
      delete r;
      r=NULL;
  }
  return res;
}

#endif
