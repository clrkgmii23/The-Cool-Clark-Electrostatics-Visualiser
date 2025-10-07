#pragma once
#define DEBUG_CONSOLE 
//#define DEBUG_FILE

#include <string>
void ErrorMessage(std::string Message);
void Info(std::string Message);

std::string ReadFile(const char* path);