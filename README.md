#C++ HTTP PROXY

##HOW TO USE
1.	Copy all the files in one directory.
2.	In the terminal:
	* cd /path/to/code/
	* make
	* `./proxy <port_number>`
3. The server should start listening for connections.
4.	Connect to the server using a web client or telnet.
5.	Send a HTTP request, like:
GET http://www.google.com/ HTTP/1.0
6.	The headers and HTML of the Google homepage will be displayed on the terminal screen.

##LIMITATIONS
*	It does not handle concurrent requests, only sequential requests.
*	Only HTTP/1.0 implemented.
*	Only “GET” method is implemented.
*	Connection: keep-alive not implemented.

##FEATURES
*	Accepts “GET” requests. Otherwise, responds back with a 501(Not Implemented) error.
*	Flexibility in choosing the server port for the server.
*	Accepts all the available formats for HTTP headers. If something goes wrong, responds with a 400(Bad Request) error.
*	Works with HTTP/1.0
Also accepts HTTP/1.1 requests, but forwards the request to remote(origin) server with HTTP version 1.0
*	Closes the client and remote server connection after a request is complete.
*	Parses the given HTTP request, separates the different parts of the requests into a map of (string, string) to make a well formatted string to be forwarded. 
*	*NOT IMPLEMENTED* Proxy works behind another proxy (without authentication). NOTE: Proxy details have to be specified in the code using a macro, file: http_proxy.cpp

##IMPLEMENTATION DETAILS
1.	`http_proxy.cpp` contains the flow of the program, i.e.,
	* Make a socket to listen to with the port number provided as an argument.
	* Accept connection from a client and receive a HTTP request.
	* Parse the request for remote server name, port (optional), path and other headers.
	* Make a well formatted request to be forwarded to the remote server.
	* Receive response from the remote server, which may even contain errors.
	* Forward the response to the client.
	* Close the connection.
	* Waits for another connection from a client. (goes back to step II)
2.	`eventLogger/eventLogger.hpp` contains the functions to log events with their timings to the standard input and standard error.
3.	`HttpHeadersParser/http_requests_parser.hpp` contains a class which handles HTTP requests. Given a request,
	* It first tokenizes the string using “\r\n” or “\r” as a delimiter.
	* Reads the first line of request for the HTTP method, URI, HTTP version.
	* Reads the subsequent lines to form a map of headers and their values.
	* Finally, forms a well formatted string of the request.
4.	`server/proxy_server.hpp` contains the functions to setup a socket and connect to it. It gracefully handles all the errors internally in making a connection. It also contains functions to receive and send message through the socket in the form of string or unsigned character (overloaded functions). It can even set a timeout on the sockets.
5.	`client/proxy_client.hpp` contains the functions to handle the part of proxy to connect to the remote server. It helps in forwarding the request and receiving the response from the remote server.
6.	`makefile` contains the required commands/dependencies to compile and make an executable.

##FLOW DIAGRAM
![Flow Diagram](http://digital-madness.in/github_img/httpproxy_flow_diagram.png)
