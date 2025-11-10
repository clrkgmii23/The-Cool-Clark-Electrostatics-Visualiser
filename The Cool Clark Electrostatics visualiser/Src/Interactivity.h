#pragma once
#include "SourceObjects.h"
#include "Camera.h"

class InteractionManager {
public:
	InteractionManager(std::vector<std::unique_ptr<ISourceObject>> &sourceObjects, int width, int height);
	~InteractionManager();
	std::vector<std::unique_ptr<ISourceObject>> &sourceObjects;
	void CreateFBO(int width, int height);
	std::vector<std::unique_ptr<Shader>> pickingShaders;
	void OnLeftClick(int x, int y, int width, int height);
	void MoveSelectedObject(float xOffset, float yOffset,int windowWidth, int windowHeight, Camera& cam);
	void Resize(int width, int height);
	unsigned int FBO, FBOtexture, FBRO;
	unsigned int selectedObject = 0;

	bool LeftMouseReleased = true;
};