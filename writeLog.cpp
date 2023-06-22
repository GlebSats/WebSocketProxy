#include "writeLog.h"
#include <fstream>

void writeLog(std::string message) {
	
	std::fstream file;

	file.open("log.txt", std::ios::app);

	if (file.is_open()) {
		file << message << std::endl;
		file.close();
	}
}
