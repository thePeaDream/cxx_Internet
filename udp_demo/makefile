.PHONY:all
all:udp_client udp_server

udp_client:udp_client.cc -lpthread
	g++ -o $@ $^ -std=c++11
udp_server:udp_server.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f udp_client udp_server