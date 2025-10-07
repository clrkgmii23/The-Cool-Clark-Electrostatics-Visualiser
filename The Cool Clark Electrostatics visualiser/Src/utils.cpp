#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

// basic debug error message
void ErrorMessage(std::string Message) {
#ifdef DEBUG_CONSOLE
	std::cout << "!> " << Message << std::endl;
#endif
#ifdef DEBUG_FILE
	// TODO: Debug into a file
#endif
}

void Info(std::string Message) {
#ifdef DEBUG_CONSOLE
	std::cout << "> " << Message << std::endl;
#endif
#ifdef DEBUG_FILE
	// TODO: Debug into a file
#endif
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