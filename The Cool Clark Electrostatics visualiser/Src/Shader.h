 #pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <memory>

class Shader {
public:
	unsigned int program = 0;
	unsigned int fragShader = 0;
	bool saveVert = false;
	unsigned int vertShader = 0;
	bool saveFrag = false;
	static unsigned int SetUpShader(const char* shaderChar, int shaderType);
	static void checkProgramError(unsigned int program);
	// default constructer because the compiler won't stop complaining about it
	Shader() = default;
	Shader(const char* vertexPath, const char* FragmentPath, bool saveVert = false, bool saveFrag = false);
	Shader(std::string vertexSource, std::string fragmentSource, bool saveVert = false, bool saveFrag = false);
	Shader(unsigned int vertexID, unsigned int fragmentID, bool saveVert = false, bool saveFrag = false);
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
	~ComputeShader();
	ComputeShader(const std::string& computeShaderSource);
	void Compute(unsigned int x, unsigned int y, unsigned int z, unsigned int BARRIER_BIT);
};


struct CommonShaders {
	std::unique_ptr<Shader> basicShader;
	std::unique_ptr<Shader> lineShader;
	std::unique_ptr<Shader> circleShader;

	CommonShaders() {
		basicShader = std::make_unique<Shader>("Src/Shaders/chargeShader.vert", "Src/Shaders/chargeShader.frag", true);
		lineShader = std::make_unique <Shader>("Src/Shaders/simpleShader.vert", "Src/Shaders/simpleShader.frag", true);
		circleShader = std::make_unique <Shader>("Src/Shaders/circleShader.vert", "Src/Shaders/circleShader.frag", true);
	}
};