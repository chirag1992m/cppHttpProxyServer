#include "proxy_client.hpp"

ProxyClient::ProxyClient() {
	this->port = std::string("80");
	this->host = "";
	socket_sd = -1;
}

ProxyClient::ProxyClient(std::string temp_host) {
	port = std::string("80");
	host = temp_host;
	socket_sd = -1;
}

ProxyClient::ProxyClient(std::string temp_host, std::string temp_port) {
	host = temp_host;
	port = temp_port;
	socket_sd = -1;
}

void ProxyClient::setHost(std::string temp_host) {
	host = temp_host;
}

void ProxyClient::setPort(std::string temp_port) {
	port = temp_port;
}

// get sockaddr, IPv4 or IPv6:
void *ProxyClient::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool ProxyClient::setAddrInfo() {
	char message[500];	//for printing error/success messages
	
	/* getting the address information for socket */
	struct addrinfo hints;
	
	memset(&hints, 0, sizeof hints);			//initializing the "hints" for getaddrinfo to empty
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;	// type of socket
	
	int rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo);

	if(rv != 0) {
		sprintf(message, "client getaddrinfo() error: %s", gai_strerror(rv));
		EventLogger::logError(message);
		return false;
	} else {
		sprintf(message, "client getaddrinfo() success: return value = %d", rv);
		EventLogger::logEvent(message);
	}
	return true;
}

bool ProxyClient::getAndConnectSocket() {
	char message[500];	//for printing error/success messages
	int rv;
	struct addrinfo *p;
	
	/* with the address structure filled with addrinfo, 
	 * we can try and setup a socket */
	for(p = servinfo; p != NULL; p = p->ai_next) {		//looping through the given addrinfo structures
		
		socket_sd = socket( p->ai_family,		//	domain(AF_INET) -> IPv4 address family
							p->ai_socktype,	//	type of socket -> stream/datagram, here stream.
							p->ai_protocol);	//	0 -> choose the proper protocol for the given type (TCP, UDP)
							
		if(socket_sd == -1) {		//checking for errors if any occured during execution of socket()
			sprintf(message, "client socket() error: %s", strerror(errno));
			EventLogger::logError(message);
			
			continue;	//try and make another socket
		} else {
			sprintf(message, "client socket() success: return value = %d", socket_sd);
			EventLogger::logEvent(message);
		}
		
		rv = connect(socket_sd, p->ai_addr, p->ai_addrlen);
		if(rv == -1) {
			sprintf(message, "client connect() error: %s", strerror(errno));
			EventLogger::logError(message);
			
			close(socket_sd);	//closing the current socket
			continue;	//try and make another socket
		} else {
			sprintf(message, "client connect() success: return value = %d", rv);
			EventLogger::logEvent(message);
		}
		
		break;		//if all the function work correcly, get out of the loop
	}
	
	if(p == NULL) {
		sprintf(message, "client addrinfo error: structure empty to set/connect any socket.");
		EventLogger::logError(message);
		
		return false;
	}
	
	char s[100];
	const char *ret = inet_ntop(p->ai_family,		//Convert Internet address format from binary to text
		get_in_addr((struct sockaddr *)p->ai_addr),
		s, sizeof s);
	if(ret == NULL) {
		sprintf(message, "client inet_ntop() error: %s", strerror(errno));
		EventLogger::logError(message);
	} else {
		sprintf(message, "Client Connecting to %s", s);
		EventLogger::logEvent(message);
	}
	
	return true;
}

bool ProxyClient::setupClient() {
	char message[500];
	if(socket_sd != -1) {
		sprintf(message, "client socket error: Another socket is already in use.");
		EventLogger::logError(message);
		
		return false;
	}
	
	/* getting the address information for socket */
	bool rv = this->setAddrInfo();
	if(!rv)
		return false;
	
	rv = this->getAndConnectSocket();
	
	freeaddrinfo(servinfo);
	if(!rv)
		return false;

	return true;
}

void ProxyClient::sendMessage(std::string msg) {
	char status[500];
	int length = msg.size();
	
	int rv = send(socket_sd, msg.data(), length, 0);
	
	if(rv == -1) {
		sprintf(status, "send() error: %s", strerror(errno));
		EventLogger::logError(status);
	} else if(rv < length) {
		std::string subMsg;
		int sent = 0, totalLength = length;
		while(rv < length && length > 0) {
			sent = rv + sent;
			subMsg = msg.substr(sent);
			
			length = length - rv;
			rv = send(socket_sd, subMsg.data(), length, 0);
			if(rv > length) {
				sprintf(status, "send() error: Extra bits sent.");
				EventLogger::logError(status);
				break;
			} else if(rv == -1) {
				sprintf(status, "send() error: %s", strerror(errno));
				EventLogger::logError(status);
				sprintf(status, "send() message: only %d bytes sent out of %d.", sent, totalLength);
				EventLogger::logError(status);
			}
		}
	} else if(rv > length) {
		sprintf(status, "send() error: Extra bits sent. total bytes sent = %d", rv);
		EventLogger::logError(status);
	}
}

std::string ProxyClient::receiveMessage(int limit) {
	char message[500];
	char buffer[5000];
	std::string finalString = "";
	int totalLength;
	
	int n = 0;
	memset(buffer, 0, sizeof buffer);
	
	if(limit == -1) {
		while((n = recv(socket_sd, buffer, 5000, 0)) > 0) {
			finalString += std::string(buffer);
			memset(buffer, 0, sizeof buffer);
			
			totalLength += n;
		}
		if(n == -1) {
			sprintf(message, "client recv() error: %s", strerror(errno));
			EventLogger::logError(message);
		}
	} else {
		while((n = recv(socket_sd, buffer, (limit > 5000) ? 5000 : limit, 0)) > 0) {
			sprintf(message, "client recveived message: %s", buffer);
			EventLogger::logEvent(message);
			
			finalString += std::string(buffer);
			memset(buffer, 0, sizeof buffer);
			
			totalLength += n;
			if(totalLength >= limit)
				break;
		}
		if(n == -1) {
			sprintf(message, "client recv() error: %s", strerror(errno));
			EventLogger::logError(message);
		}
	}
	
	return finalString;
}

/*
 * Send and receive message in an void *,
 * use of exchange of binary data */
void ProxyClient::sendMessage(unsigned char *msg, int length) {
	char status[500];
	
	int rv = send(socket_sd, msg, length, 0);
	
	if(rv == -1) {
		sprintf(status, "send() error: %s", strerror(errno));
		EventLogger::logError(status);
	} else if(rv < length) {
		
		int sent = 0, totalLength = length;
		while(rv < length && length > 0) {
			sent = rv + sent;
			
			length = length - rv;
			rv = send(socket_sd, msg + rv, length, 0);
			if(rv > length) {
				sprintf(status, "send() error: Extra bits sent.");
				EventLogger::logError(status);
				break;
			} else if(rv == -1) {
				sprintf(status, "send() error: %s", strerror(errno));
				EventLogger::logError(status);
				sprintf(status, "send() message: only %d bytes sent out of %d.", sent, totalLength);
				EventLogger::logError(status);
			}
		}
	} else if(rv > length) {
		sprintf(status, "send() error: Extra bits sent. total bytes sent = %d", rv);
		EventLogger::logError(status);
	}
}

int ProxyClient::receiveMessage(unsigned char *buf, int limit) {
	char message[500];
	int totalLength = 0, n = 0;
	
	memset(buf, 0, limit);
	
	if(limit <= 0)
		return -1;
	
	while((n = recv(socket_sd, buf + totalLength, limit - totalLength, 0)) > 0) {
		totalLength += n;
		if(totalLength > limit)
			break;
	}
	if(n == -1) {
		sprintf(message, "recv() error: %s", strerror(errno));
		EventLogger::logError(message);
	}
	
	return totalLength;
}

void ProxyClient::closeConnection() {
	close(socket_sd);
	socket_sd = -1;
}

void ProxyClient::setReceiveTimeout(unsigned int seconds) {
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	
	setsockopt (socket_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout));
}

void ProxyClient::setSendTimeout(unsigned int seconds) {
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	
	setsockopt (socket_sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout));
}
