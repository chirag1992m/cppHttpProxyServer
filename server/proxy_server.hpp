/*
 * Proxy server header file
 * Sets up a server with the given port and backlogs.
 * Listens to the port and waits for a connection
 * If any error happens in between, returns -1
*/

#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <string>
#include <iostream>
#include "../eventLogger/eventLogger.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>

//including the networking libraries
#include <netdb.h>      // definitions for network database operations
#include <arpa/inet.h>	// The arpa/inet.h header file contains definitions for internet operations.


class ProxyServer {
	private:
		std::string port;
		int backlogs;
		
		int listen_sd, accept_sd;	// socket descriptors for listening and accepting
		struct addrinfo *servinfo;	//will contain the local address(localhost) information for the socket to bind to
		struct sockaddr_storage their_addr;	//connector's address information
		int sockopt_bool;	//used for setsockopt
		std::string connector_addr;   //character array to store the address of the connector
		bool connected;
		
		bool setAddrInfo();
		bool getAndBindSocket();
		
		void *get_in_addr(struct sockaddr *);
		
	public:
		ProxyServer(std::string port, int backlogs);
		
		bool setupSocket();
		bool startListening();
		bool acceptConnection();
		
		void sendMessage(std::string);
		std::string receiveMessage(int limit = -1);
		
		void sendMessage(unsigned char *, int);
		int receiveMessage(unsigned char *, int);
		
		bool getConnected();
		
		void setReceiveTimeout(unsigned int);
		void setSendTimeout(unsigned int);
		
		void closeConnection();
		void closeSocket();
};

#endif
