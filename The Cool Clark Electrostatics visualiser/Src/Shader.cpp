#include "Shader.h"
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include "utils.h" // for file reading

// helper function to make my life easier and my code prettier
unsigned int Shader::SetUpShader(const char* shaderChar, int shaderType) {
	unsigned int shader;
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderChar, NULL);
	glCompileShader(shader);

	int success;
	std::string shaderTypeName;

	switch (shaderType){
		case GL_VERTEX_SHADER:
			shaderTypeName = "VERTEX";
			break;
		case GL_FRAGMENT_SHADER:
			shaderTypeName = "FRAGMENT";
			break;
		case GL_COMPUTE_SHADER:
			shaderTypeName = "!!!COMPUTE!!!";
			break;
	};

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		int infoLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

		std::vector<char> info(infoLength);
		glGetShaderInfoLog(shader, infoLength, NULL, info.data());
		std::string Infostr{ info.data() };
		ErrorMessage("Error While COMPILING " + shaderTypeName + " SHADER\nINFOLOG: "
			+ Infostr);
	}
	return shader;
}

void Shader::checkProgramError(unsigned int program) {
	int success;
	char info[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		int infoLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
		std::vector<char> info(infoLength);
		glGetProgramInfoLog(program, infoLength, NULL, info.data());

		std::string infoStr{ info.data() };
		ErrorMessage("Error While LINKING PROGRAM\nINFOLOG: "
			+ infoStr);
	}
}

Shader::Shader(const char* vertexPath, const char* FragmentPath,
	bool saveVert, bool saveFrag) :
	Shader(ReadFile(vertexPath), ReadFile(FragmentPath), saveVert, saveFrag) // redirect
{
}


Shader::Shader(std::string vertexSource, std::string fragmentSource, bool saveVert, bool saveFrag):
	Shader(SetUpShader(vertexSource.c_str(), GL_VERTEX_SHADER) , SetUpShader(fragmentSource.c_str(), GL_FRAGMENT_SHADER) , saveVert, saveFrag)
{

}


Shader::Shader(unsigned int vertexID, unsigned int fragmentID, bool saveVert, bool saveFrag) // program stuff
: program(0), saveVert(saveVert), saveFrag(saveFrag)
{
	vertShader = vertexID;
	fragShader = fragmentID;
	// program stuff
	program = glCreateProgram();
	glAttachShader(program, vertexID);
	glAttachShader(program, fragmentID);
	glLinkProgram(program);

	checkProgramError(program);
	if (!saveVert)
		glDeleteShader(vertexID);
	if (!saveFrag)
		glDeleteShader(fragmentID);
}


int Shader::UseProgram() const{
	glUseProgram(program);
	return program;
}

Shader::~Shader() {
	glDeleteProgram(program);
	if (saveVert)
		glDeleteShader(vertShader);
	if (saveFrag)
		glDeleteShader(fragShader);
}

// uniform stuff
void Shader::SetInt(const char* name, int value) {
	unsigned int loc = GetLoc(name);
	glUniform1i(loc, value);
}
void Shader::SetFloat(const char* name, float value) {
	unsigned int loc = GetLoc(name);
	glUniform1f(loc, value);
}
void Shader::SetBool(const char* name, bool value) {
	unsigned int loc = GetLoc(name);
	glUniform1i(loc, (int)value);
}
void Shader::SetVec2(const char* name, glm::vec2 value)
{
	unsigned int loc = GetLoc(name);
	glUniform2f(loc, value.x, value.y);
}
void Shader::SetVec3(const char* name, glm::vec3 value) {
	unsigned int loc = GetLoc(name);
	glUniform3f(loc, value.x, value.y, value.z);
}
void Shader::SetIVec3(const char* name, glm::vec3 value)
{
	unsigned int loc = GetLoc(name);
	glUniform3i(loc, value.x, value.y, value.z);
}
void Shader::SetMat4(const char* name, glm::mat4 value) {
	unsigned int loc = GetLoc(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

unsigned int Shader::GetLoc(const char* name)
{

	if (uniformCache.count(name)) {
		return uniformCache[name];
	}
	unsigned int loc = glGetUniformLocation(program, name);
	uniformCache[name] = loc;
	return loc;
}


//							    COMPUTE SHADER IMPLEMENTATION


ComputeShader::ComputeShader(const char* sourcePath): ComputeShader(ReadFile(sourcePath))
	{}

ComputeShader::ComputeShader(const std::string& computeShaderSource) {
	const char* c_computeShader = computeShaderSource.c_str();
	unsigned int computeShader = SetUpShader(c_computeShader, GL_COMPUTE_SHADER);
	program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);
	checkProgramError(program);
	glDeleteShader(computeShader);
}
void ComputeShader::Compute(unsigned int x, unsigned int y, unsigned int z, unsigned int BARRIER_BIT = GL_SHADER_STORAGE_BARRIER_BIT)
{
	this->UseProgram();
	glDispatchCompute(x, y, z);
	glMemoryBarrier(BARRIER_BIT);
}