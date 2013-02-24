/*
 * Logs events with data and time to the console(terminal).
 * author: Chirag Maheshwari
 * */
#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#include <iostream>
#include <cstdio>
#include <ctime>
#include <string>

class EventLogger {
	private:
		static std::string getCurrentTime();
	
	public:
		static void logEvent(const char *);
		
		static void logError(const char *);
};

#endif
