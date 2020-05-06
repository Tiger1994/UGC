LIB=-L /usr/local/lib/ -lhiredis -I /usr/local/restbed/distribution/include -L /usr/local/restbed/distribution/library -lrestbed

main: main.cpp ./myredis/redis.h
	g++ main.cpp -o main $(LIB)
clean:
	rm main.o main
