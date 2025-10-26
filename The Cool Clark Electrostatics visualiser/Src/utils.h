#pragma once
#define DEBUG_CONSOLE 
#include <glm/glm.hpp>

//#define DEBUG_FILE

#include <string>
void ErrorMessage(std::string Message, bool writeToFile = false);
void Info(std::string Message, bool writeToFile = false);
void Info(glm::vec3 Message, bool writeToFile = false);

std::string ReadFile(const char* path);

// useful overloads

inline std::string operator +(const std::string& str, const glm::vec3& vect) {
	return str + "x: " + std::to_string(vect.x) + ", y: " +
		std::to_string(vect.y) + ", z: " + std::to_string(vect.z);
}

inline std::string operator +(const glm::vec3& vect, const std::string& str) {
	return str + vect;
}