#pragma once

// this class will generate, compute and dispatch a compute shader to calculate the contribution for each SourceObject
// and store them in an SSBO buffer, ready to go to the renderer

#include "SourceObjects.h"
#include "Shader.h"
#include <vector>
#include <memory>

#define X_INVOCATION_NUM 16
#define Y_INVOCATION_NUM 16
#define Z_INVOCATION_NUM 1

enum visType
{
	GRID_3D,
	STREAM_LINES
};

class ComputeManager {
public:
	visType vistype;
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	glm::vec3 gridSize = glm::vec3(32);
	glm::vec3 gridGap  = glm::vec3(.1);

	int stepNum = 300;
	int pointNum = 0;

	float streamLinesdeltaTime = 0.01;
	unsigned int positionBuffer = 0; // buffer for grid vector positions
	unsigned int objectsSSBO = 0; //buffer for source objects and their properties
	unsigned int particlesSSBO = 0; //buffer particles vel and pos

	struct TypeInfo { // info for source object list
		int count;
		unsigned int bufStartPos;
		unsigned int structSize;
	};
	std::unordered_map<std::string, TypeInfo> typeInfos;

	std::unique_ptr<ComputeShader> computeShaderID;
	std::unique_ptr<ComputeShader> particleShaderID;
	std::unique_ptr<ComputeShader> SeedingcomputeShaderID;
	
	int particlesNum = 0;
	glm::vec3 particlesGap = glm::vec3(1);

	std::string fieldType = "ElectricField";

	ComputeManager(visType vistype, std::vector<std::unique_ptr<ISourceObject>>& sourceObjects);
	std::string GenerateComputeShaderSource();
	void ConfigureGrid3D(glm::vec3 _gridSize = glm::vec3(-1), glm::vec3 _gridGap = glm::vec3(-1));
	void ConfigureStreamLines(int _stepNum = -1, float _streamLinesdeltaTime = -1);
	void Compute();

	void CreateParticleShader(std::string computeShaderSource);
	void UpdateParticles(float deltaTime);
	void InitParticles(glm::vec3 particleNums, glm::vec3 particlesGap);

private:
	void grid3dSource(std::string& computeShaderSource);
	void StreamLinesSource(std::string& computeShaderSource);

	void SendSOjectsPos();
};