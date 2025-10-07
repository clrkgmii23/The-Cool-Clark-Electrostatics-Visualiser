#include <glad/glad.h>
#include "ComputeManager.h"


ComputeManager::ComputeManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, const char* computeShaderPath, unsigned int gridWidth, unsigned int gridHeight):
	sourceObjects(sourceObjects), gridWidth(gridWidth), gridHeight(gridHeight)
{
	computeShaderID = std::make_unique<ComputeShader>(computeShaderPath);
	// creating SSBO for grid positions
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	unsigned int gridSize = gridWidth*gridHeight;
	// TODO: currently this holds position 2d data for a 2d grid!
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridSize*2*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeManager::ComputeContributions()
{
	unsigned int xGroupNum = (gridWidth + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
	unsigned int yGroupNum = (gridHeight + Y_INVOCATION_NUM - 1) / Y_INVOCATION_NUM;
	computeShaderID->Compute(xGroupNum, yGroupNum,1, GL_SHADER_STORAGE_BARRIER_BIT);
}