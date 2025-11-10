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
	visType vistype; 
	unsigned int positionBuffer;
	std::vector<std::unique_ptr<ISourceObject>>& sourceObjects;
	std::unique_ptr<Shader> visualiserShader;
	glm::vec3 gridSize = glm::vec3(0);
	glm::vec3 gridGap =  glm::vec3(0);
	unsigned int gridSizeN = 0;
	unsigned int stepNum = 0;
	unsigned int pointNum = 0;
	bool dashed = true;
	Renderer(visType vistype, std::vector<std::unique_ptr<ISourceObject>>& sourceObjects,
		unsigned int positionBuffer);
	~Renderer();
	void Visualise();
	void VisualiseGrid3D();
	void ConfigureGrid3D(glm::vec3 gridSize, glm::vec3 gridGap);
	void VisualisStreamLines();
	void ConfigureStreamLines(int stepNum, int pointNum);
	void DrawShapes();

private:
	unsigned int VAO; // empty VAO since openGL requires us to bind it
};