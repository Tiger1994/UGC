LIB=-L /usr/local/lib/ -lhiredis -I /usr/local/restbed/include/ -L /usr/local/restbed/library/ -lrestbed

main: main.cpp myredis/myredis.h
	g++ main.cpp -o main $(LIB)
clean:
	rm main.o main
