#pragma once
#include "SourceObjects.h"
#include "Camera.h"
#include "Shader.h"
#include "ComputeManager.h"
#include "Renderer.h"


class InteractionManager {
public:
	InteractionManager(std::vector<std::unique_ptr<ISourceObject>> &sourceObjects, int width, int height,
		std::unique_ptr<CommonShaders> &commonShaders, std::unique_ptr<ComputeManager>& computeManager, 
		std::unique_ptr<Renderer>& renderer);
	void setPickingShader(ISourceObject& srcObj);
	~InteractionManager();
	std::vector<std::unique_ptr<ISourceObject>> &sourceObjects;
	std::unique_ptr<CommonShaders> &commonShaders;
	std::unique_ptr<ComputeManager> &computeManager;
	std::unique_ptr<Renderer> &renderer;
	void CreateFBO(int width, int height);
	std::vector<std::unique_ptr<Shader>> pickingShaders;
	void OnLeftClick(int x, int y, int width, int height);
	void onKeyPressDown(int key, int scancode, int action, int mods);
	void MoveSelectedObject(float xOffset, float yOffset,int windowWidth, int windowHeight, Camera& cam);
	template <typename objType, typename objTypeStruct>
	void AddObject(SourceObject<objType, objType> obj);
	void Resize(int width, int height);
	unsigned int FBO, FBOtexture, FBRO;
	unsigned int selectedObject= 0;
	unsigned int pickingShader = 0;

	bool LeftMouseReleased = true;
};