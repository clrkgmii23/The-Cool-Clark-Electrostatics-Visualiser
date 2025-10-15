#include <glad/glad.h>
#include "Core.h"
#include "utils.h"
#include <string>

#define GRID_WIDTH 64*2
#define GRID_HEIGHT 64*2
#define GRID_LENGTH 1

void Core::SetUp() {
	basicShader = std::make_unique<Shader>("Src/Shaders/chargeShader.vert", "Src/Shaders/chargeShader.frag");
	lineShader = std::make_unique <Shader>("Src/Shaders/infiniteLineShader.vert", "Src/Shaders/infiniteLineShader.frag");
	/*sourceObjects.push_back(std::make_unique<PointCharge>(
		glm::vec3(0.5, 0, 0)
		, 1, *basicShader));*/
	sourceObjects.push_back(std::make_unique<PointCharge>(
		glm::vec3(-0.0, 0, 0)
		, -1, *basicShader));
	sourceObjects.push_back(std::make_unique<InfiniteChargedLine>(
		glm::vec3(0.5, 0, 0)
		, 10, *lineShader));
	sourceObjects.push_back(std::make_unique<InfiniteChargedLine>(
		glm::vec3(-0.5, 0, 0)
		, -10, *lineShader));

	basicShader->UseProgram();
	basicShader->SetFloat("aspectRatio", (float)windowWidth / (float)windowHeight);

	computeManager = std::make_unique<ComputeManager>(sourceObjects, "Src/Shaders/shader.comp", GRID_WIDTH, GRID_HEIGHT, GRID_LENGTH);
	computeManager->ComputeContributions();
	// renderer
	renderer = std::make_unique<Renderer>(sourceObjects, computeManager->positionBuffer, "Src/Shaders/gridShader.vert", "Src/Shaders/gridShader.frag", 
		GRID_WIDTH, GRID_HEIGHT, GRID_LENGTH);

}

void Core::MainLoop() {
	glClearColor(0.3, 0.6, 0.1, 1.0);
	// the holy loop!!
	while (!glfwWindowShouldClose(window)) {

		double _time = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT);

		// COMPUTE PART
		sourceObjects[0]->MoveTo(glm::vec3(0.5*cos(_time), 0.1 * sin(_time), 0));
		/*sourceObjects[1]->MoveTo(glm::vec3(-0.5*cos(_time), -0.5 * sin(_time), 0));
		sourceObjects[2]->MoveTo(glm::vec3(sin(_time), 0, 0));*/
		computeManager->ComputeContributions();

		// RENDER  PART
		renderer->DrawGrid();
		renderer->DrawShapes();

		// ETC
		glfwSwapBuffers(window);
		HandleIKeyboardnput();
		glfwPollEvents();

		deltaTime = glfwGetTime() - _time;
		//Info(std::to_string(1/deltaTime));
	}
}

// initialise glfw
Core::Core(int width, int height, const char* title): 
	windowWidth(width), windowHeight(height){
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
		
	glfwSetWindowUserPointer(window, this);

	glViewport(0, 0, width, height);

	// enabling anti aliasing 
	glEnable(GL_MULTISAMPLE);

	// enabling stuff and configuing it here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	basicShader->UseProgram();
	//Info(std::to_string((float)windowWidth / (float)windowHeight));
	basicShader->SetFloat("aspectRatio", (float)windowWidth / (float)windowHeight);
}

void Core::GLFWMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	// TODO: make something with this!
}

void Core::HandleIKeyboardnput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}