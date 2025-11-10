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

enum visType
{
	GRID_3D,
	STREAM_LINES
};

class ComputeManager {
public:
	visType vistype;
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	glm::vec3 gridSize = glm::vec3(0);
	glm::vec3 gridGap  = glm::vec3(0);

	int stepNum = 60;
	int pointNum = 0;
	float streamLinesdeltaTime = 0;
	unsigned int positionBuffer = 0; // buffer for grid vector positions
	unsigned int objectsSSBO = 0; //buffer for source objects and their properties

	struct TypeInfo { // info for source object list
		int count;
		unsigned int bufStartPos;
		unsigned int structSize;
	};
	std::unordered_map<std::string, TypeInfo> typeInfos;

	std::unique_ptr<ComputeShader> computeShaderID;
	
	ComputeManager(visType vistype, std::vector<std::unique_ptr<ISourceObject>>& sourceObjects);
	std::string GenerateComputeShaderSource();

	void ConfigureGrid3D(glm::vec3 gridSize, glm::vec3 gridGap);
	void grid3dSource(std::string& computeShaderSource);

	void StreamLinesSource(std::string& computeShaderSource);
	void ConfigureStreamLines(int stepNum, float streamLinesdeltaTime);

	void SendSOjectsPos();
	void Compute();
};