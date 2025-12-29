#pragma once
#include "SourceObjects.h"
#include "Camera.h"
#include "Shader.h"
#include "ComputeManager.h"
#include "Renderer.h"
#include <GLFW/glfw3.h>


class InteractionManager {
public:
	InteractionManager(std::vector<std::unique_ptr<ISourceObject>> &sourceObjects, int width, int height,
		std::unique_ptr<CommonShaders> &commonShaders, std::unique_ptr<ComputeManager>& computeManager, 
		std::unique_ptr<Renderer>& renderer, std::unique_ptr<Camera>& cam);
	void setPickingShader(ISourceObject& srcObj);
	~InteractionManager();
	std::vector<std::unique_ptr<ISourceObject>> &sourceObjects;
	std::unique_ptr<CommonShaders> &commonShaders;
	std::unique_ptr<ComputeManager> &computeManager;
	std::unique_ptr<Camera> &cam;
	std::unique_ptr<Renderer> &renderer;
	void CreateFBO(int width, int height);
	std::vector<std::unique_ptr<Shader>> pickingShaders;
	void OnLeftClick(int x, int y, int width, int height);
	void onKeyPressDown(int key, int scancode, int action, int mods);
	void MoveSelectedObject(float xOffset, float yOffset,int windowWidth, int windowHeight, Camera& cam);
	void Resize(int width, int height);
	void ImGuiWindow(float deltaTime);
	void UpdateShader();
	unsigned int FBO, FBOtexture, FBRO;
	unsigned int selectedObject = 0;
	int lastSelectedObject = -1;
	unsigned int pickingShader = 0;
	
	// for dear imgui
	glm::vec3 particlesNumv = glm::vec3(100);
	glm::vec3 particlesGapv = glm::vec3(.01);
	float particleSize = 0.7;

	std::unordered_map<int, std::function<std::unique_ptr<ISourceObject>()>> KeyToObj{
		{GLFW_KEY_P, [this]() { return std::make_unique<PointCharge>(cam->targetPos, 1, *commonShaders->basicShader); }},
		{GLFW_KEY_C, [this]() { return std::make_unique<ChargedCircle>(cam->targetPos, 1, 1, *commonShaders->circleShader); }},
		{GLFW_KEY_L, [this]() { return std::make_unique<InfiniteChargedLine>(cam->targetPos, 1, *commonShaders->lineShader); }},
		{GLFW_KEY_R, [this]() { return std::make_unique<InfiniteChargedCylinder>(cam->targetPos, 1, 1, *commonShaders->cylinderShader); }},
		{GLFW_KEY_S, [this]() { return std::make_unique<ChargedSphere>(cam->targetPos, 1, 1, *commonShaders->sphereShader); }}
	};

	bool LeftMouseReleased = true;
	void SetLeftMouseRelease(bool val);
	bool showVis = true;
	bool timePlay = false;
	int selectedField = 0;
	glm::vec3 lockAxis = glm::vec3(1);
};