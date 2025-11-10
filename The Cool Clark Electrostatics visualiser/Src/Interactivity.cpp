#include "Interactivity.h"

InteractionManager::InteractionManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, int width, int height):sourceObjects(sourceObjects)
{
	std::string fragmentString = ReadFile("Src/Shaders/PickingShader.frag");
	const char* fragmentchar = fragmentString.c_str();

	unsigned int pickingShader = Shader::SetUpShader(fragmentchar, GL_FRAGMENT_SHADER);

	for (size_t i = 0; i < sourceObjects.size(); i++)
	{
		// i --> Shader
		if (sourceObjects[i]->GetShader()->vertShader == 0) {
			ErrorMessage("Object With i = " + std::to_string(i) + " Didn't Save It's VERTEX shader");
		}
		pickingShaders.push_back(std::make_unique<Shader>(sourceObjects[i]->GetShader()->vertShader, pickingShader));
	}
	CreateFBO(width, height);
}

InteractionManager::~InteractionManager()
{
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &FBOtexture);
	glDeleteRenderbuffers(1, &FBRO);
}

void InteractionManager::CreateFBO(int width, int height) {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// texture for color
	glGenTextures(1, &FBOtexture);
	glBindTexture(GL_TEXTURE_2D, FBOtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtexture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// render object for depth and stencil
	glGenRenderbuffers(1, &FBRO);
	glBindRenderbuffer(GL_RENDERBUFFER, FBRO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,FBRO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		ErrorMessage("Could Not Generate Frame Buffer");
		return;
	}
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InteractionManager::OnLeftClick(int x, int y, int width, int height) {
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glLineWidth(7); // so it's easier to select line objects
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (size_t i = 0; i < sourceObjects.size(); i++)
	{
		pickingShaders[i]->UseProgram();
		pickingShaders[i]->SetInt("id", i+1);
		sourceObjects[i]->Draw(false);
	}

	unsigned char pixels[4] ;
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	y = height - y;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	selectedObject = float(pixels[0]) + float(pixels[1]) * 255 + float(pixels[2]) * 255 * 255;
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back
	LeftMouseReleased = false;
}

void InteractionManager::MoveSelectedObject(float xOffset, float yOffset, int windowWidth, int windowHeight, Camera& cam)
{
	if (LeftMouseReleased || selectedObject == 0) return;
	// fov is constant at 70d
	float dist = glm::distance(sourceObjects[selectedObject - 1]->GetPos(), cam.camPos);
	float PixelToWorld = 2 * dist * tan(glm::radians(35.0f)) / (float)windowHeight;

	glm::vec3 moveOffset = (xOffset * cam.camRight + yOffset * cam.camUp)* PixelToWorld;

	sourceObjects[selectedObject - 1]->AddPos(moveOffset);
}

void InteractionManager::Resize(int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, FBOtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA8, GL_UNSIGNED_BYTE, NULL); // huh, 800x600 fixed... hmmm
	glBindRenderbuffer(GL_RENDERBUFFER, FBRO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
