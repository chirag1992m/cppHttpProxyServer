
#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

#include <string>
#include <cstring>
#include "../eventLogger/eventLogger.hpp"
#include <cstdio>
#include <cerrno>

#include <netdb.h>
#include <arpa/inet.h>

class ProxyClient {
	private:
		std::string port;
		std::string host;
		
		int socket_sd;
		struct addrinfo *servinfo;	//will contain the local address(localhost) information for the socket to bind to
		
		void *get_in_addr(struct sockaddr *);
		bool setAddrInfo();
		bool getAndConnectSocket();
		
	public:
		ProxyClient();
		ProxyClient(std::string);
		ProxyClient(std::string, std::string);
		
		void setHost(std::string);
		void setPort(std::string);
		
		bool setupClient();
		void closeConnection();
		
		void sendMessage(std::string);
		std::string receiveMessage(int limit = -1);
		
		void sendMessage(unsigned char *, int);
		int receiveMessage(unsigned char *, int);
		
		void setReceiveTimeout(unsigned int);
		void setSendTimeout(unsigned int);
};

#endif
