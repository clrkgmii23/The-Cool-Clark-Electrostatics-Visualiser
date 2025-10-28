 #pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class Shader {
public:
	unsigned int program;
	Shader() {}; // default constructer because the compiler won't stop complaining about it
	Shader(const char* vertexPath, const char* FragmentPath);
	~Shader();
	int UseProgram() const;
	// uniform setters
	void SetInt(const char* name, int value);
	void SetFloat(const char* name, float value);
	void SetBool(const char* name, bool value);
	void SetVec2(const char* name, glm::vec2 value);
	void SetVec3(const char* name, glm::vec3 value);
	void SetIVec3(const char* name, glm::vec3 value);
	void SetMat4(const char* name, glm::mat4 value);

private:
	unsigned int GetLoc(const char* name);
	std::unordered_map<std::string, unsigned int> uniformCache;
};

class ComputeShader : public Shader{
public:
	// kind of a dumb design, if you pass you pass a const char, it considers it a path
	// if you pass a string, it considers it a source
	ComputeShader(const char* sourcePath);
	ComputeShader(const std::string& computeShaderSource);
	void Compute(unsigned int x, unsigned int y, unsigned int z, unsigned int BARRIER_BIT);
};