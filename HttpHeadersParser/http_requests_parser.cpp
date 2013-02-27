#include "http_requests_parser.hpp"
#include <iostream>
void HttpRequestsParser::setRequest(std::string request) {
	requestString = request;
	formattedString = "";
	parsed = false;
}

bool HttpRequestsParser::getParsed() {
	return parsed;
}

void HttpRequestsParser::setStatus(int stat) {
	status = stat;
}

bool HttpRequestsParser::setMethod(std::string &temp_method) {
	upperCase(temp_method);
	for(method = 1; method < 9; method++) {
		if(temp_method.compare(getMethodName()) == 0)
			return true;
	}
	
	return false;
}

bool HttpRequestsParser::setVersion(std::string &temp_version) {
	
	upperCase(temp_version);
	for(version = 1; version < 3; version++) {
		if(temp_version.compare(getVersionString()) == 0)
			return true;
	}
	
	return false;
}

bool HttpRequestsParser::setURI(std::string &temp_uri) {
	//std::cout<<"urihere"<<std::endl;
	machineName = "";
	relativePath = "";
	machinePort = "";
	std::string protocol;
	
	if(temp_uri.size() < 1)
		return false;
	//Checking if the given uri is an absolute path or a relative one
	if(temp_uri[0] == '/'){	//relative path
		relativePath = temp_uri;
		return true;
		
	} else {	//absolute path
	//std::cout<<"urihere"<<std::endl;
		size_t end;
		end = temp_uri.find(std::string("://"));
		if(end == std::string::npos)
			return false;
		else {
			protocol = temp_uri.substr(0, end+1);
			lowerCase(protocol);
		}
	//std::cout<<"urihere"<<std::endl;
		if(protocol.compare(std::string("http")) == 0)
			return false;
		else {
	//std::cout<<"urihere"<<std::endl;
			std::string hierarchicalPart = temp_uri.substr(end+3, std::string::npos);
			end = hierarchicalPart.find(":", 0);
			
			if(end != std::string::npos) {
				machineName = hierarchicalPart.substr(0, end);
				
				size_t end_url = hierarchicalPart.find("/", 0);
				
				machinePort = hierarchicalPart.substr(end+1, end_url-end-1);
				relativePath = hierarchicalPart.substr(end_url, std::string::npos);
			} else {
				end = hierarchicalPart.find("/", 0);
				machineName = hierarchicalPart.substr(0, end);
				relativePath = hierarchicalPart.substr(end, std::string::npos);
				machinePort = std::string("80");
			}
		}
	}
	
	return true;
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

void HttpRequestsParser::upperCase(std::string &strToConvert) {
	for(unsigned int i=0;i<strToConvert.length();i++) {
		strToConvert[i] = toupper(strToConvert[i]);
	}
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
	formattedString += getVersionString();
		formattedString += std::string("\r\n");
	
	//making the other lines
	std::map<std::string ,std::string>::iterator it;
	for(it = headers.begin(); it != headers.end(); ++it) {
		//if(it->second.compare("keep-alive") == 0)
			//continue;
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
		pos = requestString.find("\r");
		if(pos == std::string::npos)
			return setBadRequest();
		else
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
	//std::cout<<"here"<<std::endl;
	if(lines[0][0] == ' ')		//checking if the first character is a space or not which is not allowed in a intial line of request
		return setBadRequest();
	if(words.size() != 3) {		//exactly 3 parts in the request are necessary
		return setBadRequest();
	} else {
		//std::cout<<"here"<<std::endl;
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
	//std::cout<<"here"<<std::endl;
	/*
	 * Parsing all the other headers given */
	if(lines.size() > 1) {
		//std::cout<<"here"<<std::endl;
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
	//std::cout<<"here"<<std::endl;
	std::map<std::string ,std::string>::iterator it;
	it = headers.find(std::string("host"));
	if(machineName.size() == 0) {
		if(it != headers.end()) {
			machineName = it->second;
			size_t end = machineName.find(":", 0);
			
			if(end != std::string::npos) {
				machinePort = machineName.substr(end+1, std::string::npos);
				machineName = machineName.substr(0, end);
			} else {
				machinePort = std::string("80");
			}
		}
		else {
			//std::cout<<"machine name not found!";
			return setBadRequest();
		}
	} else {
		if(it == headers.end())
			headers.insert(std::pair<std::string, std::string>(std::string("host"), machineName+std::string(":")+machinePort));
	}
	//std::cout<<"here"<<std::endl;
	//Got a 200-OK request
	//making a formatted requested.
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

std::string HttpRequestsParser::getMachinePort() {
	return machinePort;
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
	return machineName+std::string(":")+machinePort+relativePath;
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
