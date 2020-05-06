#include "redis.h"

int main(){
	Redis *r=new Redis();
	if(!r->connect("127.0.0.1",6379)){
		printf("connection error!\n");
		return 0;
	}
	r->set("name","Tiger");
	printf("Get the name is %s\n",r->get("name").c_str());
	delete r;r=NULL;
	return 0;
}
