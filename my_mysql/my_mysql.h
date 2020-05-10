#ifndef UGC_MY_MYSQL_MY_MYSQL_H_
#define UGC_MY_MYSQL_MY_MYSQL_H_

#include "mysql++.h"
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void create_table_in_mysql() {
  const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
	mysqlpp::Connection conn(db, server, user, pass);
  std::string cmd = "CREATE TABLE IF NOT EXISTS resource( \
                      resource_id INT NOT NULL AUTO_INCREMENT, \
                      title VARCHAR(100) NOT NULL, \
                      author VARCHAR(100) NOT NULL, \
                      keywords VARCHAR(100) NOT NULL, \
                      time DATE NOT NULL, \
                      content MEDIUMTEXT NOT NULL, \
                      PRIMARY KEY (resource_id) \
                     )ENGINE=InnoDB DEFAULT CHARSET=utf8;";
  mysqlpp::Query query = conn.query(cmd.c_str());
	query.execute();
}

void insert_record_to_mysql(std::string &cmd) {
	const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
	mysqlpp::Connection conn(db, server, user, pass);
	mysqlpp::Query query = conn.query(cmd.c_str());
	query.execute();
}

void select_one_day_from_mysql(std::string &cmd, rapidjson::Document &json_doc){
	const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
	mysqlpp::Connection conn(db, server, user, pass);
	mysqlpp::Query query = conn.query(cmd.c_str());
	mysqlpp::StoreQueryResult res = query.store();
	
	raidjson::StringBuffer str_buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
	writer.StartObject();
	for (size_t i = 0; i < res.size(); i++) {
		writer.Key(res[i]["id"]);
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
#endif
