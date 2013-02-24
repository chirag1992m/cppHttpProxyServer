#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include "eventLogger/eventLogger.hpp"
#include "server/proxy_server.hpp"
#include "HttpHeadersParser/http_requests_parser.hpp"
#include "client/proxy_client.hpp"

using namespace std;


/* global variables */
string PORT = "";	//port for socket to connect to

int BACKLOG = 5;	//maximum number of listeners on the listening port at an instant

int main(int argc, char *argv[]) {
	char message[500];
	
	/* Getting port and backlogs(optional) from the command line */
	switch(argc) {
		case 1: {		// no arguments given - using default port
			sprintf(message, "PORT error: no PORT specified. Using default port number of 1234.");
			EventLogger::logError(message);
			PORT = "1234";
			break;
		}
		case 2: {		//extracting the port number from command line
			int temp_port = atoi(argv[1]);
			if(temp_port < 1024) {
				sprintf(message, "PORT error: port given is not an integer or less than 1024.");
				EventLogger::logError(message);
				
				return -1;
			} else if(temp_port > 65534){
				sprintf(message, "PORT error: port given too big (i.e.) greater than 65534.");
				EventLogger::logError(message);
				
				return -1;
			} else {
				string temp;
				while(temp_port > 0) {
					temp += temp_port%10 + 48;
					temp_port /= 10;
				}
				for (unsigned int i=0;i<temp.length();i++)
					PORT += temp[temp.length()-i-1];
			}
			break;
		}
		case 3:{	//extracting backlogs from command line
			int temp_backlog = atoi(argv[1]);
			if(temp_backlog < 1) {
				sprintf(message, "BACKLOG error: number of backlogs specified are too small (<1). using default backlog of 5.");
				EventLogger::logError(message);
			} else if(temp_backlog > 15) {
				sprintf(message, "BACKLOG error: number of backlogs specified are too large (>15). using default backlog of 5.");
				EventLogger::logError(message);
			} else {
				BACKLOG = temp_backlog;
			}
			break;
		}
		default:	//incorrect format of argument specification
			sprintf(message, "Argument error: Format to start ./proxy <port-number> <backlog(optional)>");
			EventLogger::logError(message);
			
			return -1;
			break;
	}
	
	/* Setting up of server */
	sprintf(message, "Starting Server at port: %s", PORT.c_str());
	EventLogger::logEvent(message);
	
	ProxyServer server(PORT, BACKLOG);
	HttpRequestsParser parser;
	ProxyClient client;
	
	if(server.setupSocket()) {	//setting up of socket
		sprintf(message, "Setting up of server sucess.");
		EventLogger::logEvent(message);
	} else {
		sprintf(message, "Coudln\'t set up the server.");
		EventLogger::logError(message);
		
		return -1;
	}
	
	if(!server.startListening()) {	//start listening to the socket
		sprintf(message, "Can't listen to the socket that set-up.");
		EventLogger::logError(message);
		
		return -1;
	}
	
	bool acceptConnections = true;
	int tries = 0;
	
	while(acceptConnections && tries < 5) {
		parser.clear();
		if(!server.acceptConnection()) {
			sprintf(message, "Couldn't accept connection.");
			EventLogger::logError(message);
			
			sprintf(message, "Trying again. Try number: %d. Max tries available: 5", ++tries);
			EventLogger::logEvent(message);
			
			continue;
		}
		
		//counting the number of connections made
		tries = 0;
		
		/* sending welcome message */
//		server.sendMessage(string("Welcome to HTTP_PROXY\nSend \"\\r\\n\\r\\n\" to complete request\n"));
		
		/* receiving requests from client */
		//server.setReceiveTimeout(180);
		string received = server.receiveMessage();
		
		sprintf(message, "Message(requests) received from client:");
		EventLogger::logEvent(message);
		EventLogger::logEvent((string("\"") + received + string("\"")).c_str());
		
		
		sprintf(message, "Parsing of given request starting.");
		EventLogger::logEvent(message);
		
		/* checking if there is a exit request from the client */
		if(received.compare(string("quitServer\r\n")) == 0) {
			sprintf(message, "Server shutdown requested!");
			EventLogger::logEvent(message);
			acceptConnections = false;
		}
		
		/* If not a exit request, parsing the request further for http requests */
		int status;
		if(acceptConnections) {
			parser.setRequest(received);
			status = parser.parse();
			
			sprintf(message, "Parsing of request completed!");
			EventLogger::logEvent(message);
			EventLogger::logEvent(parser.getStatusString().c_str());
			
			if(status > 0) {		//if a not implemented or bad request is found
				server.sendMessage(parser.getStatusString() + string("\r\n"));
			} else {
				sprintf(message, "Formatted request:");
				EventLogger::logEvent(message);
				EventLogger::logEvent((string("\r\n") + parser.getFormattedRequest()).c_str());
				
				sprintf(message, "Connecting to host through ProxyClient");
				EventLogger::logEvent(message);
				client.setHost(parser.getHostName());
				client.setPort(parser.getMachinePort());
				if(!client.setupClient()) {
					sprintf(message, "Couldn't connect to given host.");
					EventLogger::logError(message);
					
					server.sendMessage(string("Name Error: The Host name does not exist or is not reachable."));
				}
				else {
					int length = 0, maxLength = 500000;
					unsigned char buf[maxLength+1];
					
					sprintf(message, "Connection to host successful!");
					EventLogger::logEvent(message);
					
					//forwarding requests to host
					sprintf(message, "Forwarding requests to host!");
					EventLogger::logEvent(message);
					client.sendMessage(parser.getFormattedRequest());
					
					//taking reply from host
					//client.setReceiveTimeout(5);
					do {
						sprintf(message, "Receiving reply from host!");
						EventLogger::logEvent(message);
						length = client.receiveMessage(buf, maxLength);
						sprintf(message, "%d length of reply received.", length);
						EventLogger::logEvent(message);
						
						//forwarding reply to client
						sprintf(message, "Forwarding reply from host to client.");
						EventLogger::logEvent(message);
						server.sendMessage(buf, length);
					} while(length > 0);
					
					client.closeConnection();
				}
			}
		}
		
		sprintf(message, "closing currently connected client.");
		EventLogger::logEvent(message);
		server.closeConnection();
	}
	
	sprintf(message, "Shutting down the server! Good bye :)");
	EventLogger::logEvent(message);
	
	server.closeSocket();
	return 0;
}
