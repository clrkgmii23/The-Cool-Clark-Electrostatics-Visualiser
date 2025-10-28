#pragma once

// this class is responsible for everything Render related, from drawing the sourceObjects
// to rendering the grid or any other visualisation type.
// it will take a sourceObjects list to render the objects,
// and an SSBO buffer to render the grid

#include "SourceObjects.h"
#include <vector>
#include <memory>


class Renderer {
public:
	unsigned int positionBuffer;
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	std::unique_ptr<Shader> gridShader;
	glm::vec3 gridSize;
	glm::vec3 gridGap;
	unsigned int gridSizeN;
	Renderer(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
		unsigned int positionBuffer, const char* vertexShaderPath, const char* fragmentShaderPath,
		glm::vec3 gridSize, glm::vec3 gridGap);
	~Renderer();
	void DrawGrid();
	void DrawShapes();

private:
	unsigned int VAO; // empty VAO since openGL requires us to bind it
};