#ifndef UGC_MY_MYSQL_MY_MYSQL_H_
#define UGC_MY_MYSQL_MY_MYSQL_H_

#include <string>

#include "mysql++.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void CreateTableInMysql() {
  const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
  mysqlpp::Connection conn(db, server, user, pass);
  std::string cmd = "CREATE TABLE IF NOT EXISTS resource( \
                      id INT NOT NULL AUTO_INCREMENT, \
                      title VARCHAR(100) NOT NULL, \
                      author VARCHAR(100) NOT NULL, \
                      keywords VARCHAR(100) NOT NULL, \
                      time VARCHAR(20) NOT NULL, \
                      content MEDIUMTEXT NOT NULL, \
                      PRIMARY KEY (id) \
                     )ENGINE=InnoDB DEFAULT CHARSET=utf8;";
  mysqlpp::Query query = conn.query(cmd.c_str());
  query.execute();
}

void SelectOneDayFromMysql(const std::string &date, const std::string str_limit, \
    const std::string str_offset, std::string &records){
  int limit = 10, offset = 0;
  if(!str_limit.empty()) limit = std::stoi(str_limit);
  if(!str_offset.empty()) offset = std::stoi(str_offset);

  const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
  mysqlpp::Connection conn(false);
  conn.set_option(new mysqlpp::SetCharsetNameOption("utf8"));
  conn.connect(db, server, user, pass);
  
  std::string cmd = "SELECT * FROM resource WHERE time=\"" + date +"\";";
  mysqlpp::Query query = conn.query(cmd.c_str());
  mysqlpp::StoreQueryResult res = query.store();

  std::string res_num = "Get " + std::to_string(res.size()) + " records";
  fprintf(stdout, "%.*s\n", res_num.size(), res_num.c_str()); 
  rapidjson::StringBuffer str_buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
  writer.StartObject();
  writer.Key("total");
  writer.Int(res.size());
  int ceil = offset + limit > res.size() ? res.size() : offset + limit;
  for (size_t i = offset; i < ceil; i++) {
    std::string str_index = std::to_string(i);
    writer.Key(str_index.c_str());
    writer.StartObject();
    writer.Key("title");
    writer.String(res[i]["title"]);
    writer.Key("author");
    writer.String(res[i]["author"]);
    writer.Key("keywords");
    writer.String(res[i]["keywords"]);
    writer.Key("time");
    writer.String(res[i]["time"]);
    writer.Key("content");
    writer.String(res[i]["content"]);
    writer.EndObject();
  }
  writer.EndObject();
  records = str_buf.GetString();
}

void WriteRecordToMysql(const rapidjson::Document &json_doc) {
  std::string show_status = "Write to MySQL.";
  fprintf(stdout, "%.*s\n", (int)show_status.size(), show_status.c_str());

  std::string cmd;
  std::stringstream scmd;
  scmd << "INSERT INTO resource (title, author, keywords, time, content) VALUES (\"";
  scmd << json_doc["title"].GetString() << "\", \"" <<  \
         json_doc["author"].GetString() << "\", \"" << \
         json_doc["keywords"].GetString() << "\", \"" << \
         json_doc["time"].GetString() << "\", \"" << \
         json_doc["content"].GetString() << "\");";
  cmd = scmd.str();
  //fprintf(stdout, "%.*s\n", (int)cmd.size(), cmd.c_str());
  
  const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
  mysqlpp::Connection conn(false);
  conn.set_option(new mysqlpp::SetCharsetNameOption("utf8"));
  conn.connect(db, server, user, pass);
  mysqlpp::Query query = conn.query(cmd.c_str());
  query.execute();
}

#endif
