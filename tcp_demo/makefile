.PHONY:all
all:tcp_server 
tcp_server:tcp_server.cc
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -f tcp_server