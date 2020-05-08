#ifndef UGC_MY_MYSQL_MY_MYSQL_H_
#define UGC_MY_MYSQL_MY_MYSQL_H_

#include "mysql++"
#include <string>
#include "rapidjson/document.h"

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

	Document::AllocatorType& allocator = json_doc.GetAllocator();
	json_doc.AddMember("title", res[0]["title"], allocator);
	json_doc.AddMember("author", res[0]["author"], allocator);
	json_doc.AddMember("keywords", res[0]["keywords"], allocator);
	json_doc.AddMember("time", res[0]["time"], allocator);
	json_doc.AddMember("content", res[0]["content"], allocator);
}
#endif
