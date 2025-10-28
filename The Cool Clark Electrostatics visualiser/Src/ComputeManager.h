#pragma once

// this class will generate and compute and dispatch a compute shader to calculatecontribution for each SourceObject
// and store them in an SSBO buffer, ready to go to the renderer

#include "SourceObjects.h"
#include "Shader.h"
#include <vector>
#include <memory>

#define X_INVOCATION_NUM 16
#define Y_INVOCATION_NUM 16
#define Z_INVOCATION_NUM 1

class ComputeManager {
public:
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	glm::vec3 gridSize;
	glm::vec3 gridGap;
	unsigned int positionBuffer; // buffer for grid vector positions
	unsigned int objectsSSBO; //buffer for source objects and their properties

	struct TypeInfo { // info for source object list
		int count;
		unsigned int bufStartPos;
		unsigned int structSize;
	};
	std::unordered_map<std::string, TypeInfo> typeInfos;

	std::unique_ptr<ComputeShader> computeShaderID;
	
	ComputeManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, const char* computeShaderPath, glm::vec3 gridSize, glm::vec3 gridGap);
	std::string GenerateComputeShaderSource();
	void SendSOjectsPos();
	void ComputeContributions();
};