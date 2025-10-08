#include "Renderer.h"
#include "Shader.h"
#include "utils.h"


Renderer::Renderer(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
	unsigned int positionBuffer, const char* vertexShaderPath, const char* fragmentShaderPath,
	unsigned int gridWidth, unsigned int gridHeight, unsigned int gridLength) :
	sourceObjects(sourceObjects), gridWidth(gridWidth), gridHeight(gridHeight), gridLength(gridLength),
	positionBuffer(positionBuffer),
	VAO(0)
{
	gridShader = std::make_unique<Shader>(vertexShaderPath, fragmentShaderPath);
	gridSize = gridWidth * gridHeight * gridLength;

	gridShader->UseProgram();
	gridShader->SetInt("gridWidth", gridWidth);
	gridShader->SetInt("gridHeight", gridHeight);
	gridShader->SetInt("gridLength", gridLength);

	glGenVertexArrays(1, &VAO);
}

Renderer::~Renderer()
{
	glDeleteBuffers(1, &VAO);
}

void Renderer::DrawShapes()
{
	// just loop and draw
	for (auto& SObject : sourceObjects)
	{
		SObject->Draw();
		Info(std::to_string(SObject->GetPos().x));
	}
}

void Renderer::DrawGrid()
{
	gridShader->UseProgram();
	glBindVertexArray(VAO);
	// draw a 2d vector, so gridsize * 2 for tail and head of the vector
	glDrawArraysInstanced(GL_LINES, 0, 2, gridSize*2);
	glBindVertexArray(0);
}