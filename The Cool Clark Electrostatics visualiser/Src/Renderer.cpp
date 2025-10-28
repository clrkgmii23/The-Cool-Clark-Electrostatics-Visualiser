#include "Renderer.h"
#include "Shader.h"
#include "utils.h"


Renderer::Renderer(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
	unsigned int positionBuffer, const char* vertexShaderPath, const char* fragmentShaderPath,
	glm::vec3 gridSize, glm::vec3 gridGap) :
	sourceObjects(sourceObjects), gridSize(gridSize),
	positionBuffer(positionBuffer), gridGap(gridGap),
	VAO(0)
{
	gridShader = std::make_unique<Shader>(vertexShaderPath, fragmentShaderPath);
	gridSizeN = gridSize.x* gridSize.y * gridSize.z;

	gridShader->UseProgram();
	gridShader->SetIVec3("gridSize", gridSize);
	gridShader->SetVec3("gridGap", gridGap);

	glGenVertexArrays(1, &VAO);
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &VAO);
}

void Renderer::DrawShapes()
{
	// just loop and draw
	for (auto& SObject : sourceObjects)
	{
		SObject->Draw();
	}
}

void Renderer::DrawGrid()
{
	gridShader->UseProgram();
	glBindVertexArray(VAO);
	// draw grid with given positionBuffer and grid dimentions
	glDrawArraysInstanced(GL_LINES, 0, 2, gridSizeN);
	glBindVertexArray(0);
}