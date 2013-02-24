#include "proxy_server.hpp"

ProxyServer::ProxyServer(std::string port, int backlogs) {
	this->port = port;
	this->backlogs = backlogs;
	
	this->sockopt_bool = 1;
	
	this->listen_sd = -1;
	this->accept_sd = -1;
	
	this->connected = false;
}

bool ProxyServer::setAddrInfo() {
	char message[500];	//for printing error/success messages
	
	/* getting the address information for socket */
	struct addrinfo hints;
	
	memset(&hints, 0, sizeof hints);			//initializing the "hints" for getaddrinfo to empty
	hints.ai_family = AF_INET;			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// type of socket
	hints.ai_flags = AI_PASSIVE;		// use my IP
	
	int rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo); //get the address-information for localhost.
        //NULL -> specifies a loop-back
        //getaddrinfo uses "hints" to fill "servinfo" with the required info (hints send the required parameters)

	if(rv != 0) {
		sprintf(message, "getaddrinfo() error: %s", gai_strerror(rv));
		EventLogger::logError(message);
		return false;
	} else {
		sprintf(message, "getaddrinfo() success: return value = %d", rv);
		EventLogger::logEvent(message);
	}
	return true;
}

bool ProxyServer::getAndBindSocket() {
	char message[500];	//for printing error/success messages
	int rv;
	struct addrinfo *p;
	
	if(listen_sd != -1) {
		sprintf(message, "socket error: Another socket is already in use.");
		EventLogger::logError(message);
		
		return false;
	}
	
	/* with the address structure filled with addrinfo, 
	 * we can try and setup a socket */
	for(p = servinfo; p != NULL; p = p->ai_next) {		//looping through the given addrinfo structures
		
		listen_sd = socket( p->ai_family,		//	domain(AF_INET) -> IPv4 address family
							p->ai_socktype,	//	type of socket -> stream/datagram, here stream.
							p->ai_protocol);	//	0 -> choose the proper protocol for the given type (TCP, UDP)
							
		if(listen_sd == -1) {		//checking for errors if any occured during execution of socket()
			sprintf(message, "socket() error: %s", strerror(errno));
			EventLogger::logError(message);
			
			continue;	//try and make another socket
		} else {
			sprintf(message, "socket() success: return value = %d", listen_sd);
			EventLogger::logEvent(message);
		}
		
		/*
		 * setting the socket to be reusable, i.e., it can be binded again and again without error
		 * when not in use */
		
		rv = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &sockopt_bool, sizeof(int));
		if(rv == -1) {
			sprintf(message, "getsockopt() error: %s", strerror(errno));
			EventLogger::logError(message);
			
			return false;
		} else {
			sprintf(message, "getsockopt() success: return value = %d", rv);
			EventLogger::logEvent(message);
		}
		
		rv = bind(listen_sd, p->ai_addr, p->ai_addrlen);
		if(rv == -1) {
			sprintf(message, "bind() error: %s", strerror(errno));
			EventLogger::logError(message);
			
			close(listen_sd);	//closing the current socket
			continue;	//try and make another socket
		} else {
			sprintf(message, "bind() success: return value = %d", rv);
			EventLogger::logEvent(message);
		}
		
		break;		//if all the function work correcly, get out of the loop
	}
	
	if(p == NULL) {
		sprintf(message, "addrinfo error: structure empty to set/bind any socket.");
		EventLogger::logError(message);
		
		return false;
	}
	
	return true;
}

bool ProxyServer::setupSocket() {
	/* getting the address information for socket */
	bool rv = this->setAddrInfo();
	if(!rv)
		return false;
	
	rv = this->getAndBindSocket();
	
	freeaddrinfo(servinfo);
	if(!rv)
		return false;
	
	//freeing up the address info structure
	
	return true;
}

bool ProxyServer::startListening() {
	char message[500];
	
	int rv = listen(listen_sd, backlogs);
	if(rv == -1) {
		sprintf(message, "listen() error: %s", strerror(errno));
		EventLogger::logError(message);
		
		return false;
	} else {
		sprintf(message, "listen() success: return value = %d", rv);
		EventLogger::logEvent(message);
		
		return true;
	}
}

// get sockaddr, IPv4 or IPv6:
void *ProxyServer::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool ProxyServer::acceptConnection() {	//blocking function
	char message[500], s[100];	//for printing error/success messages
	socklen_t sin_size = sizeof their_addr;
	
	if(accept_sd != -1) {
		sprintf(message, "acceptConnection() error: accepting multiple connections at the same time not allowed");
		EventLogger::logError(message);
		
		return false;
	}
	
	sprintf(message, "accepting connections on port: %s", port.c_str());
	EventLogger::logEvent(message);
	accept_sd = accept(listen_sd, (struct sockaddr *)&their_addr, &sin_size);
	
	if(accept_sd == -1) {
		sprintf(message, "accept() error: %s", strerror(errno));
		EventLogger::logError(message);
		
		return false;
	}
	
	connected = true;
	sprintf(message, "accept() success: return value = %d", accept_sd);
	EventLogger::logEvent(message);
	
	const char *rv = inet_ntop(their_addr.ss_family,		//Convert Internet address format from binary to text
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
	if(rv == NULL) {
		sprintf(message, "inet_ntop() error: %s", strerror(errno));
		EventLogger::logError(message);
	} else {
		sprintf(message, "Got connection from %s", s);
		EventLogger::logEvent(message);
		
		connector_addr = std::string(s);
	}
	
	return true;
}

void ProxyServer::sendMessage(std::string msg) {
	char status[500];
	int length = msg.size();
	
	int rv = send(accept_sd, msg.data(), length, 0);
	
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
			rv = send(accept_sd, subMsg.data(), length, 0);
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

//receives a string until after an actual message, double enter(\r\n\r\n) is pressed
std::string ProxyServer::receiveMessage(int limit) {
	char message[500];
	char buffer[5000];
	std::string finalString = "", tempString = "";
	int totalLength;
	
	int n = 0;
	bool MsgFound = false;
	memset(buffer, 0, sizeof buffer);
	
	if(limit == 0)
		return finalString;
	else if(limit > 0) {
		totalLength = 0;
		while((n = recv(accept_sd, buffer, 5000, 0)) > 0) {
			finalString += std::string(buffer);
			memset(buffer, 0, sizeof buffer);
			
			totalLength += n;
			if(totalLength > limit)
				break;
		}
		if(n == -1) {
			sprintf(message, "recv() error: %s", strerror(errno));
			EventLogger::logError(message);
		}
	} else {
		size_t find1, find2;
		while((n = recv(accept_sd, buffer, 5000, 0)) > 0) {
			//std::cout<<"n="<<n<<"  "<<buffer<<std::endl;
			if(n > 2)
				MsgFound = true;
			
			if(n == 2 && MsgFound)
				break;
			
			tempString = std::string(buffer);
			
			if(n != 2 || MsgFound)
				finalString += tempString;
			
			find1 = tempString.find("\r\r");
			find2 = tempString.find("\r\n\r\n");
			
			if(find1 != std::string::npos || find2 != std::string::npos)
				break;
			
			memset(buffer, 0, sizeof buffer);
		}
		if(n == -1) {
			sprintf(message, "recv() error: %s", strerror(errno));
			EventLogger::logError(message);
		}
	}
	
	return finalString;
}

/*
 * Send and receive message in an void *,
 * use of exchange of binary data */
void ProxyServer::sendMessage(unsigned char *msg, int length) {
	char status[500];
	
	int rv = send(accept_sd, msg, length, 0);
	
	if(rv == -1) {
		sprintf(status, "send() error: %s", strerror(errno));
		EventLogger::logError(status);
	} else if(rv < length) {
		
		int sent = 0, totalLength = length;
		while(rv < length && length > 0) {
			sent = rv + sent;
			
			length = length - rv;
			rv = send(accept_sd, msg + rv, length, 0);
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

int ProxyServer::receiveMessage(unsigned char *buf, int limit) {
	char message[500];
	int totalLength = 0, n = 0;
	
	memset(buf, 0, limit);
	
	if(limit <= 0)
		return -1;
	
	while((n = recv(accept_sd, buf + totalLength, limit - totalLength, 0)) > 0) {
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


bool ProxyServer::getConnected() {
	return this->connected;
}

void ProxyServer::setReceiveTimeout(unsigned int seconds) {
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	
	setsockopt (accept_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout));
}

void ProxyServer::setSendTimeout(unsigned int seconds) {
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	
	setsockopt (accept_sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout));
}

void ProxyServer::closeConnection() {
	close(accept_sd);
	accept_sd = -1;
	connected = false;
}

void ProxyServer::closeSocket() {
	close(listen_sd);
	listen_sd = -1;
}
