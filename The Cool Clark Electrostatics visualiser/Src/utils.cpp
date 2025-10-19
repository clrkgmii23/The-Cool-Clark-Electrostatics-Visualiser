#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

// basic debug error message
void AppendIntoFile(const char* filePath, std::string content) {
	std::fstream logFile;
	logFile.open(filePath, std::ios::out | std::ios::app);

	if (!logFile.is_open()) {
		std::cout << "!> Could NOT open LOG file for writing!" << std::endl;
		return;
	}

	logFile << content << std::endl;
}

// super simple function to return file content as a string
std::string ReadFile(const char* path) {
	std::ifstream fileStream(path);
	if (!fileStream.is_open()) {
		ErrorMessage("Could NOT open file with PATH: " + std::string(path));
	}

	std::stringstream strStream;
	strStream << fileStream.rdbuf();
	return strStream.str();
}
bool logFileOpen = false;

void LogFile(std::string Message) {
	if (logFileOpen == false) {
		// clear the log file first
		logFileOpen = true;
		AppendIntoFile("logFile.log", "----------------------------------------------------------------------------------");
	}
	AppendIntoFile("logFile.log", Message);
}

void ErrorMessage(std::string Message, bool writeToFile) {
#ifdef DEBUG_CONSOLE
	std::cout << "!> " << Message << std::endl;
#endif
#ifdef DEBUG_FILE
	if(writeToFile)
		LogFile("!> " + Message);
#endif
}

void Info(std::string Message, bool writeToFile) {
#ifdef DEBUG_CONSOLE
	std::cout << "> " << Message << std::endl;
#endif
#ifdef DEBUG_FILE
	if (writeToFile)
		LogFile("> " + Message);
#endif
}