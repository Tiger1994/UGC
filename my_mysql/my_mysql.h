#ifndef UGC_MY_MYSQL_MY_MYSQL_H_
#define UGC_MY_MYSQL_MY_MYSQL_H_

#include "mysql++.h"
#include <string>
#include "rapidjson/document.h"

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

void select_one_record_from_mysql(std::string &cmd, rapidjson::Document &json_doc){
	const char* db = "tiger", *server = "localhost", *user = "root", *pass = "123";
	mysqlpp::Connection conn(db, server, user, pass);
	mysqlpp::Query query = conn.query(cmd.c_str());
	mysqlpp::StoreQueryResult res = query.store();

  std::string title(res[0]["title"]);
  std::string author(res[0]["author"]);
  std::string keywords(res[0]["keywords"]);
  std::string time(res[0]["time"]);
  std::string content(res[0]["content"]);
  rapidjson::Document::AllocatorType& allocator = json_doc.GetAllocator();
	json_doc.AddMember("title", rapidjson::StringRef(title.c_str()), allocator);
	json_doc.AddMember("author", rapidjson::StringRef(author.c_str()), allocator);
	json_doc.AddMember("keywords", rapidjson::StringRef(keywords.c_str()), allocator);
	json_doc.AddMember("time", rapidjson::StringRef(time.c_str()), allocator);
	json_doc.AddMember("content", rapidjson::StringRef(content.c_str()), allocator);
}
#endif
