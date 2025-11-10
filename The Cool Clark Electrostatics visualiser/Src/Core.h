#pragma once
#include <vector>
#include <memory>
#include "SourceObjects.h"
#include "ComputeManager.h"
#include "Renderer.h"
#include "Camera.h"
#include "Interactivity.h"
#include <GLFW/glfw3.h>

// this class is going to serve as the bridge between most the components
// of this project. it will handle setting up glfw, handling input, 
// initialising stuff and storing the source objects to hand
// them to the Renderer and ComputeManager

class Core {
public:
	void SetUp();
	void MainLoop();
	Core(int width, int height, const char* title);
	~Core();

	double deltaTime = 0.0;

	std::unique_ptr<Shader> basicShader;
	std::unique_ptr<Shader> lineShader;
	std::unique_ptr<Shader> circleShader;

	std::unique_ptr<ComputeManager> computeManager;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Camera> mainCam;
	std::unique_ptr<InteractionManager> interactionManager;
	// long ass type name
	std::vector<std::unique_ptr<ISourceObject>> sourceObjects;

private:
	GLFWwindow* window;
	unsigned int MatUBO;
	void SetUpUniformBuffer();
	void UpdateView();
	void UpdatePres();
	unsigned int windowWidth;
	unsigned int windowHeight;
	static void GLFWErrorCallback(int error, const char* description);
	static void GLFWWindowSizeCallback(GLFWwindow* window, int width, int height);
	void GLFWWindowSizeCallbackBounce(GLFWwindow* window, int width, int height);
	static void GLFWMouseCallback(GLFWwindow* window, double xpos, double ypos); // moving
	void GLFWMouseCallbackBounce(GLFWwindow* window, double xpos, double ypos);
	static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods); // clicking
	void GLFWMouseButtonCallbackBounce(GLFWwindow* window, int button, int action, int mods);
	static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void GLFWScrollCallbackBounce(GLFWwindow* window, double xoffset, double yoffset);
	void HandleIKeyboardInput();

};