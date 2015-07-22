CC=g++
CFLAGS=-std=c++11
CXXFLAGS=-std=c++11

all: server cmd-client random-client

clean:
	rm -f *~ server cmd-client random-client

server: server.cpp
	$(CC) $(CFLAGS) server.cpp -lpthread -lboost_system -lboost_thread -o server

cmd-client: cmd-client.cpp
	$(CC) $(CFLAGS) cmd-client.cpp -lpthread -lboost_system -lboost_thread -o cmd-client

random-client: random-client.cpp
	$(CC) $(CFLAGS) random-client.cpp -lpthread -lboost_system -lboost_thread -o random-client

