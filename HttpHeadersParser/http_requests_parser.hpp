/*
 * Get a string and parses it
 * Stores the different headers given in a map of
 * string (header name) and string (header value)
 * It also stores the http version requested and the method
 * 
 * It validates the requests and returns the corresponding status
 * of the request,
 * 400 - Bad Request
 * 501 - Not Implemented
 * 200 - OK
 * 
 * */
 
#ifndef HTTP_REQUESTS_PARSER_H
#define HTTP_REQUESTS_PARSER_H

#include <cstring>
#include <string>
#include <vector>
#include <map>
//#include <regex>

class HttpRequestsParser {
	private:
		std::string requestString;
		std::string formattedString;
		
		std::map<std::string, std::string> headers;
		int method;
		std::string relativePath;
		std::string machineName;
		int version;
		
		int status;
		bool parsed;
		
		
		void tokenizeString(std::string &, std::vector<std::string> &, std::string &);
		void setStatus(int);
		
		int setBadRequest();
		int setNotImplemented();
		
		bool setMethod(std::string &);
		bool setVersion(std::string &);
		bool setURI(std::string &);
		void makeFormattedRequest();
		
		void lowerCase(std::string &);
		void trim(std::string &);
		
	public:
		HttpRequestsParser() {
			requestString = "";
			formattedString = "";
			parsed = false;
		}
		
		HttpRequestsParser(std::string request) {
			requestString = request;
			formattedString = "";
			parsed = false;
		}
		
		void setRequest(std::string);
		
		int parse();
		
		bool getParsed();
		std::string getFormattedRequest();
		std::string getHostName();
		std::string getStatusString();
		std::string getVersionString();
		std::string getMethodName();
		std::string getFullURL();
		
		void clear();
};

#endif
