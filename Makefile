LIB=-L /usr/local/lib/ -lhiredis -I /usr/local/restbed/include/ -L /usr/local/restbed/library/ -lrestbed -I/usr/include/mysql -I /usr/local/include/mysql++/ -lmysqlpp

main: main.cpp myredis/myredis.h my_mysql/my_mysql.h
	g++ main.cpp -o main $(LIB)
clean:
	rm main.o main
