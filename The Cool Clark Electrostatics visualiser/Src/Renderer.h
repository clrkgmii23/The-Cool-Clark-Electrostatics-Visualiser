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
	unsigned int gridWidth, gridHeight, gridLength;
	unsigned int gridSize;
	Renderer(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
		unsigned int positionBuffer, const char* vertexShaderPath, const char* fragmentShaderPath,
		unsigned int gridWidth, unsigned int gridHeight, unsigned int gridLength);
	~Renderer();
	void DrawGrid();
	void DrawShapes();

private:
	unsigned int VAO; // empty VAO since openGL requires us to bind it
};