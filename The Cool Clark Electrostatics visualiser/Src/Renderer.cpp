#include "ComputeManager.h"
#include "Renderer.h"
#include "Shader.h"
#include "utils.h"


Renderer::Renderer(visType vistype, std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
	unsigned int positionBuffer) : vistype(vistype),sourceObjects(sourceObjects),
	positionBuffer(positionBuffer),
	VAO(0) {
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

void Renderer::Visualise()
{
	switch (vistype) {
	case GRID_3D:
		VisualiseGrid3D();
		break;
	case STREAM_LINES:
		VisualisStreamLines();
		break;
	default:
		ErrorMessage("Incorrect Visualising Type In Renderer");
		break;
	}
}

void Renderer::VisualiseGrid3D()
{
	visualiserShader->UseProgram();
	glBindVertexArray(VAO);
	// draw grid with given positionBuffer and grid dimentions
	glDrawArraysInstanced(GL_LINES, 0, 2, gridSizeN);
	glBindVertexArray(0);
}

void Renderer::VisualisStreamLines()
{
	visualiserShader->UseProgram();
	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_LINES, 0, stepNum, pointNum);
	glBindVertexArray(0);
}

void Renderer::ConfigureGrid3D(glm::vec3 _gridSize = glm::vec3(10), glm::vec3 _gridGap = glm::vec3(0.5))
{
	gridSize = _gridSize;
	gridGap  = _gridGap;
	visualiserShader = std::make_unique<Shader>("Src/Shaders/gridShader.vert", "Src/Shaders/gridShader.frag");
	gridSizeN = gridSize.x * gridSize.y * gridSize.z;

	visualiserShader->UseProgram();

	visualiserShader->SetIVec3("gridSize", gridSize);
	visualiserShader->SetVec3("gridGap", gridGap);
}

void Renderer::ConfigureStreamLines(int _stepNum, int _pointNum) {
	stepNum = _stepNum;
	pointNum = _pointNum;

	visualiserShader = std::make_unique<Shader>("Src/Shaders/streamLines.vert", "Src/Shaders/streamLines.frag");

	visualiserShader->UseProgram();

	visualiserShader->SetInt("stepNum", stepNum);
	visualiserShader->SetInt("pointNum", pointNum);
}