#Specifying the compiler
CC=g++
#options during compilation
#add: -std=c++0x for <regex>
CFLAGS=-Wall -c 

proxy: eventLogger.o proxy_server.o http_proxy.o http_requests_parser.o proxy_client.o
	$(CC) http_proxy.o eventLogger.o proxy_server.o http_requests_parser.o proxy_client.o -o proxy

http_proxy.o: http_proxy.cpp
	$(CC) $(CFLAGS) http_proxy.cpp

eventLogger.o: eventLogger/eventLogger.cpp eventLogger/eventLogger.hpp
	$(CC) $(CFLAGS) eventLogger/eventLogger.cpp

proxy_server.o: server/proxy_server.cpp server/proxy_server.hpp
	$(CC) $(CFLAGS) server/proxy_server.cpp

http_requests_parser.o: HttpHeadersParser/http_requests_parser.cpp HttpHeadersParser/http_requests_parser.hpp
	$(CC) $(CFLAGS) HttpHeadersParser/http_requests_parser.cpp

proxy_client.o: client/proxy_client.hpp client/proxy_client.cpp
	$(CC) $(CFLAGS) client/proxy_client.cpp

clean:
	rm -rf *.o
