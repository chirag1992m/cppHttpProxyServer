#include "eventLogger.hpp"

std::string EventLogger::getCurrentTime() {
	char a[50];
	
	std::string timer = "";
	time_t now = time(0);
	tm *ltm = localtime(&now);
	
	sprintf(a, "%d/%d/%d %d:%d:%d"
					, ltm->tm_mday, 1+ ltm->tm_mon, 1900+ltm->tm_year
					,ltm->tm_hour, ltm->tm_min, 1 + ltm->tm_sec);
	
	timer += std::string(a);
	
	return timer;
}

void EventLogger::logEvent(const char *message) {	
	std::cout<<getCurrentTime()<<" - "<<message<<std::endl;
}

void EventLogger::logError(const char *message) {		
	std::cerr<<getCurrentTime()<<" - "<<message<<std::endl;
}
