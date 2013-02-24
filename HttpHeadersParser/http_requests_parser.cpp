#include "http_requests_parser.hpp"

void HttpRequestsParser::setRequest(std::string request) {
	requestString = request;
	formattedString = "";
	parsed = false;
}

bool HttpRequestsParser::getParsed() {
	return parsed;
}

void HttpRequestsParser::tokenizeString(std::string &theString, std::vector<std::string> &tokens, std::string &delimiter) {
	if(delimiter.size() <= 0 || theString.size() <= 0)
		return;
	
	size_t start = 0, end = 0;
	while(end != std::string::npos) {
		end = theString.find(delimiter, start);
		
		tokens.push_back(theString.substr(start, 
			(end == std::string::npos) ? std::string::npos : end-start));
		
		start = ((end > (std::string::npos - delimiter.size())) ? std::string::npos : end+delimiter.size());
		
		if(tokens.back().compare(delimiter) == 0 || tokens.back().size() == 0) {
			tokens.pop_back();
		}
	}
}

void HttpRequestsParser::setStatus(int stat) {
	status = stat;
}

bool HttpRequestsParser::setMethod(std::string &temp_method) {
	
	for(method = 1; method < 9; method++) {
		if(temp_method.compare(getMethodName()) == 0)
			return true;
	}
	
	return false;
}

bool HttpRequestsParser::setURI(std::string &temp_uri) {
	machineName = "";
	relativePath = "";
	
	if(temp_uri.size() < 1)
		return false;
	//Checking if the given uri is an absolute path or a relative one
	if(temp_uri[0] == '/'){	//relative path
		relativePath = temp_uri;
		return true;
		
	} else {	//absolute path
		if(temp_uri.substr(0, 7).compare(std::string("http://")) != 0 && temp_uri.substr(0, 7).compare(std::string("HTTP://")) != 0)
			return false;
		else {
			std::string hierarchicalPart = temp_uri.substr(7, std::string::npos);
			size_t end = hierarchicalPart.find("/", 0);
			
			machineName = hierarchicalPart.substr(0, end);
			relativePath = hierarchicalPart.substr(end, std::string::npos);
		}
	}
	
	return true;
}

bool HttpRequestsParser::setVersion(std::string &temp_version) {
	
	for(version = 1; version < 3; version++) {
		if(temp_version.compare(getVersionString()) == 0)
			return true;
	}
	
	return false;
}

int HttpRequestsParser::setBadRequest() {
	setStatus(2);
	parsed = true;
	return status;
}

int HttpRequestsParser::setNotImplemented() {
	setStatus(1);
	parsed = true;
	return status;
}

void HttpRequestsParser::lowerCase(std::string &strToConvert) {
	for(unsigned int i=0;i<strToConvert.length();i++) {
		strToConvert[i] = tolower(strToConvert[i]);
	}
}

void HttpRequestsParser::trim(std::string &strToTrim) {
	for(size_t i=0; i<strToTrim.length(); ++i) {
		if(strToTrim[i] == ' ' || strToTrim[i] == '\t')
			strToTrim.erase(i, 1);
		else
			break;
	}
	
	for(size_t i=strToTrim.length()-1; i>=0; --i) {
		if(strToTrim[i] == ' ' || strToTrim[i] == '\t')
			strToTrim.erase(i, 1);
		else
			break;
	}
}

void HttpRequestsParser::makeFormattedRequest() {
	//making the initialLine
	formattedString += getMethodName();
		formattedString += std::string(" ");
	formattedString += relativePath;
		formattedString += std::string(" ");
	version = 1;
	formattedString += getVersionString();
		formattedString += std::string("\r\n");
	
	//making the other lines
	std::map<std::string ,std::string>::iterator it;
	for(it = headers.begin(); it != headers.end(); ++it) {
		formattedString += it->first;
			formattedString += std::string(":");
		formattedString += it->second;
			formattedString += std::string("\r\n");
	}
	formattedString += std::string("\r\n");
}

int HttpRequestsParser::parse() {
	if(parsed)
		return status;
	
	if(requestString.length() < 1)
		return setBadRequest();
	//first checking if delimited by \r or \r\n
	size_t pos = requestString.find("\r\n");
	if(pos == std::string::npos) {
		//std::cout<<"replacing!!!!!"<<std::endl<<std::endl;
		requestString.replace(requestString.begin(), requestString.end(), "\r", "\r\n");
	} else {
		//std::cout<<"found!!!!!"<<std::endl<<std::endl;
	}
	
	std::vector<std::string> lines, words;
	
	std::string delimiter = std::string("\r\n");
	tokenizeString(requestString, lines, delimiter);
	
	if(lines.size() < 1)
		return setBadRequest();
	
	/*
	 * parsing the initial line (getting the type of method, url(relative/absolute), version) 
	*/
	delimiter = std::string(" ");
	tokenizeString(lines[0], words, delimiter);
	
	if(lines[0][0] == ' ')		//checking if the first character is a space or not which is not allowed in a intial line of request
		return setBadRequest();
	if(words.size() != 3) {		//exactly 3 parts in the request are necessary
		return setBadRequest();
	} else {
		bool temp = setMethod(words[0]);
		if(!temp) {
//			std::cout<<"wrong method!";
			return setBadRequest();
		}
		//checking for not implemented
		if(method != 2)
			return setNotImplemented();
		
		temp = setURI(words[1]);
		if(!temp) {
//			std::cout<<"wrong URI!";
			return setBadRequest();
		}
		
		temp = setVersion(words[2]);
		if(!temp) {
//			std::cout<<"wrong version!";
			return setBadRequest();
		}
	}
	
	/*
	 * Parsing all the other headers given */
	if(lines.size() > 1) {
		size_t end = 0;
		std::string headerName, headerVal;
		//parsing the first header
		if(lines[1][0] == '\t' || lines[1][0] == ' ') {
			//std::cout<<"first header has space";
			return setBadRequest();
		}
		end = lines[1].find(":", 0);
		if(end == std::string::npos)
			return setBadRequest();
		headerName = lines[1].substr(0, end);
		headerVal = lines[1].substr(end+1, std::string::npos);
		lowerCase(headerName);
		
		for(unsigned int i=2; i<lines.size(); i++) {
			if(lines[i][0] == '\t' || lines[i][0] == ' ') {
				headerVal += lines[i];
			} else {
				trim(headerVal);
				headers.insert(std::pair<std::string, std::string>(headerName, headerVal));
				
				end = lines[i].find(":", 0);
				if(end == std::string::npos)
					return setBadRequest();
				headerName = lines[i].substr(0, end);
				headerVal = lines[i].substr(end+1, std::string::npos);
				lowerCase(headerName);
			}
		}
		trim(headerVal);
		headers.insert(std::pair<std::string, std::string>(headerName, headerVal));
	}
	
	std::map<std::string ,std::string>::iterator it;
	it = headers.find(std::string("host"));
	if(machineName.size() == 0) {
		if(it != headers.end())
			machineName = it->second;
		else {
			//std::cout<<"machine name not found!";
			return setBadRequest();
		}
	} else {
		if(it == headers.end())
			headers.insert(std::pair<std::string, std::string>(std::string("host"), std::string(machineName)));
	}
	
	//Got a 200-OK request
	//making a formatted requested to be forwarded to the server
	makeFormattedRequest();
	
	setStatus(0);
	parsed = true;
	return status;
}


std::string HttpRequestsParser::getFormattedRequest() {
	return formattedString;
}

std::string HttpRequestsParser::getHostName() {
	return machineName;
}

std::string HttpRequestsParser::getVersionString() {
	switch(version) {
		case 1:	return std::string("HTTP/1.0");
		case 2:	return std::string("HTTP/1.1");
	}
	return std::string("");
}

std::string HttpRequestsParser::getStatusString() {
	switch(status) {
		case 0: return std::string("200 OK");
		case 1: return std::string("501 Not Implemented");
		case 2: return std::string("400 Bad Request");
	}
	return std::string("");
}

std::string HttpRequestsParser::getMethodName() {
	switch(method) {
		case 1:	return std::string("OPTIONS");
		case 2:	return std::string("GET");
		case 3:	return std::string("HEAD");
		case 4:	return std::string("POST");
		case 5:	return std::string("PUT");
		case 6:	return std::string("DELETE");
		case 7:	return std::string("TRACE");
		case 8:	return std::string("CONNECT");
	}
	
	return std::string("");
}

std::string HttpRequestsParser::getFullURL() {
	return machineName+relativePath;
}

void HttpRequestsParser::clear() {
	headers.clear();
	formattedString = "";
	machineName = "";
	relativePath = "";
	method = 0;
	version = 0;
	parsed = false;
	status = -1;
	requestString = "";
}
