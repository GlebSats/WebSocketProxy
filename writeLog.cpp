#include "writeLog.h"
#include <fstream>
#include <ctime>
#include <Windows.h>
#include <string>

void writeLog(std::string message) {
	
	TCHAR Path[MAX_PATH];

	GetModuleFileName(NULL, Path, MAX_PATH);

	std::wstring logPath = Path;
	size_t i = logPath.find_last_of(L"\\");
	logPath = logPath.substr(0, i + 1) + L"log.txt";
	
	std::fstream file;

	file.open(logPath, std::ios::app);

	if (file.is_open()) {
		std::time_t currentTime = std::time(nullptr);
		std::tm localTime;
		localtime_s(&localTime, &currentTime);

		char dateTime[20];
		std::strftime(dateTime, sizeof(dateTime), "%d-%m-%Y %H:%M:%S", &localTime);

		file << "[" << dateTime << "] " << message << std::endl;
		
		file.close();
	}
}
