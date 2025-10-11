#pragma once

// this class will manage computing the contribution for each SourceObject, and store them in
// an SSBO buffer
// TODO: possibly generate the compute shader with this?

#include "SourceObjects.h"
#include "Shader.h"
#include <vector>
#include <memory>

// this should match what's in shader.comp
#define X_INVOCATION_NUM 16
#define Y_INVOCATION_NUM 16
#define Z_INVOCATION_NUM 1

class ComputeManager {
public:
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	unsigned int gridWidth, gridHeight, gridLength;
	unsigned int positionBuffer; // buffer for grid vector positions
	unsigned int objectsSSBO; //buffer for source objects and their properties

	struct TypeInfo { // info for source object list
		int count;
		unsigned int bufStartPos;
		unsigned int structSize;
	};
	std::unordered_map<std::string, TypeInfo> typeInfos;

	std::unique_ptr<ComputeShader> computeShaderID;
	
	ComputeManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, const char* computeShaderPath, unsigned int gridWidth, unsigned int gridHeight, unsigned int gridLength);
	std::string GenerateComputeShaderSource();
	void ComputeContributions();
};