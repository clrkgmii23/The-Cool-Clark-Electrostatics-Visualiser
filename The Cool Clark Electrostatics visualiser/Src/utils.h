#pragma once
#define DEBUG_CONSOLE 
//#define DEBUG_FILE

#include <string>
void ErrorMessage(std::string Message, bool writeToFile = false);
void Info(std::string Message, bool writeToFile = false);

std::string ReadFile(const char* path);