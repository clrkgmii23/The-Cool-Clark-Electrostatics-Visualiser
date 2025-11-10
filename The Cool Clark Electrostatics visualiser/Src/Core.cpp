#include <glad/glad.h>
#include "Core.h"
#include "utils.h"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define VIS_TYPE STREAM_LINES

#define GRID_WIDTH  32
#define GRID_HEIGHT 32
#define GRID_LENGTH 32

#define GRID_GAP_X .1
#define GRID_GAP_Y .1
#define GRID_GAP_Z .1

#define STEP_SIZE 400
#define STREAM_LINE_STEP_TIME float(1)/60

void Core::SetUp() {
	mainCam = std::make_unique<Camera>();
	mainCam->x_rot_sen = 0.7;
	mainCam->y_rot_sen = 0.7;
	// shaders
	basicShader = std::make_unique<Shader>("Src/Shaders/chargeShader.vert", "Src/Shaders/chargeShader.frag", true);
	lineShader = std::make_unique <Shader>("Src/Shaders/simpleShader.vert", "Src/Shaders/simpleShader.frag", true);
	circleShader = std::make_unique <Shader>("Src/Shaders/circleShader.vert", "Src/Shaders/circleShader.frag", true);

	// add objects
	sourceObjects.push_back(std::make_unique<PointCharge>(
		glm::vec3(-0.5, 0, 0 )
		, 1, *basicShader));
	sourceObjects[0]->seedNum = 50;
	sourceObjects.push_back(std::make_unique<ChargedCircle>(
		glm::vec3(0, 0, 1.5)
		, -1, 1, *circleShader));
	sourceObjects.push_back(std::make_unique<ChargedCircle>(
		glm::vec3(0, 0, 3.5)
		, -5, .4, *circleShader));

	// compute
	computeManager = std::make_unique<ComputeManager>(VIS_TYPE, sourceObjects);
	// renderer
	renderer = std::make_unique<Renderer>(computeManager->vistype, sourceObjects, computeManager->positionBuffer);
	if (VIS_TYPE == GRID_3D) {
		computeManager->ConfigureGrid3D(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_LENGTH), glm::vec3(GRID_GAP_X, GRID_GAP_Y, GRID_GAP_Z));
		renderer->ConfigureGrid3D(computeManager->gridSize, computeManager->gridGap);
	}
	
	else if (VIS_TYPE == STREAM_LINES){
		computeManager->ConfigureStreamLines(STEP_SIZE, STREAM_LINE_STEP_TIME);
		renderer->ConfigureStreamLines(computeManager->stepNum, computeManager->pointNum);
		//renderer->dashed = false;
	}

	// interaction
	interactionManager = std::make_unique<InteractionManager>(sourceObjects, windowWidth, windowHeight);
	computeManager->Compute(); // comment it in mainLoop if you don't want it to update each frame
	SetUpUniformBuffer();
	UpdatePres();

}

void Core::MainLoop() {
	// the holy loop!!
	while (!glfwWindowShouldClose(window)) {

		glLineWidth(1);
		glClearColor(0.3, 0.6, 0.1, 1.0);
		double _time = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//
		
		//sourceObjects[0]->MoveTo(glm::vec3(.5 * cos(_time),   .5*sin(_time), sin(_time)));
		//sourceObjects[1]->MoveTo(glm::vec3(-.5 * cos(_time), -.5 * sin(_time),cos(_time)));

		UpdateView();

		// COMPUTE PART
		computeManager->Compute();

		// RENDER  PART
		renderer->Visualise();
		renderer->DrawShapes();

		// ETC
		glfwSwapBuffers(window);
		HandleIKeyboardInput();
		glfwPollEvents();

		deltaTime = glfwGetTime() - _time;
		//Info(std::to_string(1/deltaTime)); // see frames
	}
}

// initialise glfw
Core::Core(int width, int height, const char* title): 
	windowWidth(width), windowHeight(height), MatUBO(0){
	if (width < 1 || height < 1 || title == nullptr) {
		ErrorMessage("Invalid inputs when creating CORE");
	}

	// set up glfw
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4); // anti aliasing

	// create window
	window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window) {
		ErrorMessage("Could Not Create GLFW WINDOW");
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);

	// load glad functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ErrorMessage("Could Not Set Up GLAD");
		glfwTerminate();
	}

	// some cool coolbacks
	glfwSetErrorCallback(GLFWErrorCallback);
	glfwSetWindowSizeCallback(window, GLFWWindowSizeCallback);
	glfwSetCursorPosCallback(window, GLFWMouseCallback);
	glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
	glfwSetScrollCallback(window, GLFWScrollCallback);


	glfwSetWindowUserPointer(window, this);

	glViewport(0, 0, width, height);

	// enabling anti aliasing 
	glEnable(GL_MULTISAMPLE);

	// enabling stuff and configuing it here
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Core::SetUpUniformBuffer() {
	glGenBuffers(1, &MatUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, MatUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, MatUBO);
}

void Core::UpdateView()
{
	glBindBuffer(GL_UNIFORM_BUFFER, MatUBO);
	glm::mat4 view = mainCam->GetViewMatrix();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
}
void Core::UpdatePres()
{
	glm::mat4 prespective = glm::perspective(glm::radians(70.0f), (float)windowWidth / (float)windowHeight, 0.001f, 1000.0f);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(prespective));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


Core::~Core()
{

}
//					---------- the callbacks ----------
void Core::GLFWErrorCallback(int error, const char* description)
{
	ErrorMessage("GLFW ERROR: " + std::string(description));
}

// bounce function for windowSize
void Core::GLFWWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	Core* myCore = (Core*)glfwGetWindowUserPointer(window);
	if (myCore)
		myCore->GLFWWindowSizeCallbackBounce(window, width, height);
}

void Core::GLFWWindowSizeCallbackBounce(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	if (windowHeight < 1) windowHeight = 1;
	if (windowWidth < 1) windowWidth = 1;

	UpdatePres();

	interactionManager->Resize(width, height);
}

float lastX = 0;
float lastY = 0;

void Core::GLFWMouseCallback(GLFWwindow* window, double xpos, double ypos) {
	Core* myCore = (Core*)glfwGetWindowUserPointer(window);
	if(myCore)
		myCore->GLFWMouseCallbackBounce(window, xpos, ypos);
}

void Core::GLFWMouseCallbackBounce(GLFWwindow* window, double xpos, double ypos)
{
	float xOffset = xpos - mainCam->lastX;
	float yOffset = mainCam->lastY - ypos;
	interactionManager->MoveSelectedObject(xOffset, yOffset,windowWidth,  windowHeight, *mainCam);

	//if (glfwGetMouseButton(window, 0) == GLFW_PRESS) // left clicking
	if(glfwGetMouseButton(window, 1) == GLFW_PRESS) // right clicking
		mainCam->HandleMouseInput(xOffset, -yOffset, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

	mainCam->lastX = (float)xpos;
	mainCam->lastY = (float)ypos;
}

void Core::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Core* myCore = (Core*)glfwGetWindowUserPointer(window);
	if (myCore)
		myCore->GLFWMouseButtonCallbackBounce(window, button, action, mods);
}

void Core::GLFWMouseButtonCallbackBounce(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { // left click
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		interactionManager->OnLeftClick(xpos, ypos, windowWidth, windowHeight);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){ // left release

		interactionManager->LeftMouseReleased = true;
	}
}

void Core::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Core* myCore = (Core*)glfwGetWindowUserPointer(window);
	if (myCore)
		myCore->GLFWScrollCallbackBounce(window, xoffset, yoffset);
}

void Core::GLFWScrollCallbackBounce(GLFWwindow* window, double xoffset, double yoffset){
	mainCam->HandleScroll(xoffset, yoffset);
}

void Core::HandleIKeyboardInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}