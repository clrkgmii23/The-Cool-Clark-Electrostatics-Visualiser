#include "Interactivity.h"
#include <functional>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

// CONTROLS:
//	-    MOUSE:	shift right drag -> move camera
//	-	      :	right drag -> rotate camera
//	-	      :	mouse scroll -> zoom/unzoom camera
//	-		  :	left drag on object -> move object
//  - KEYBOARD: shift {P OR L OR C OR R OR S} -> add source object
//  -         : {X OR Y OR Z} -> lock into axis
//  -         : control {X OR Y OR Z} -> lock into plane
//  -         : SPACE -> stop time
//  -         : TAB   -> show/hide visualisation
//  -         : D   -> delete slected

InteractionManager::InteractionManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, int width, int height,
	std::unique_ptr<CommonShaders>& commonShaders, std::unique_ptr<ComputeManager>& computeManager,
	std::unique_ptr<Renderer>& renderer, std::unique_ptr<Camera>& cam):sourceObjects(sourceObjects), commonShaders(commonShaders),
	computeManager(computeManager),renderer(renderer), cam(cam)
{
	std::string fragmentString = ReadFile("Src/Shaders/PickingShader.frag");
	const char* fragmentchar = fragmentString.c_str();

	pickingShader = Shader::SetUpShader(fragmentchar, GL_FRAGMENT_SHADER);

	for (size_t i = 0; i < sourceObjects.size(); i++)
	{
		setPickingShader(*sourceObjects[i]);
	}
	CreateFBO(width, height);
}

void InteractionManager::setPickingShader(ISourceObject& srcObj) {
	// i --> Shader
	if (srcObj.GetShader()->vertShader == 0) {
		ErrorMessage("Object" + srcObj.typeID + ": "  + std::to_string(srcObj.uniqueId)+ " Didn't Save It's VERTEX shader");
	}
	pickingShaders.push_back(std::make_unique<Shader>(srcObj.GetShader()->vertShader, pickingShader));
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
	glLineWidth(8); // so it's easier to select line objects
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

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		return;
	}
	// below zero currsponds to clicking nothing
	if(selectedObject > 0) lastSelectedObject = selectedObject;
}

void InteractionManager::onKeyPressDown(int key, int scancode, int action, int mods)
{
	// this is a temporary solution, since we don't have a UI!
	// adding objects
	for (const auto &pair: KeyToObj)
	{
		if (key == pair.first && action == GLFW_PRESS && mods == GLFW_MOD_SHIFT) {
			sourceObjects.push_back(pair.second());
			setPickingShader(*sourceObjects[sourceObjects.size() - 1]);
			//sourceObjects[sourceObjects.size() - 1]->seedNum = 5; // give it a default seed so it looks interesting

			UpdateShader();
		}
	}

	// axis locking
	if		(key == GLFW_KEY_X && action == GLFW_PRESS) lockAxis = glm::vec3(1,0,0);
	else if (key == GLFW_KEY_Y && action == GLFW_PRESS) lockAxis = glm::vec3(0,1,0);
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS) lockAxis = glm::vec3(0,0,1);
	// plane locking
	if (key == GLFW_KEY_X && action == GLFW_PRESS && (mods == GLFW_MOD_CONTROL)) lockAxis = glm::vec3(0, 1, 1);
	else if (key == GLFW_KEY_Y && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) lockAxis = glm::vec3(1,0,1);
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) lockAxis = glm::vec3(1,1,0);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) timePlay = !timePlay;
	auto io = ImGui::GetIO();
	if (key == GLFW_KEY_TAB   && action == GLFW_PRESS && !io.WantCaptureKeyboard) showVis= !showVis;

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		// delete the object, also can't delete if there is only one object! won't fix!
		if (sourceObjects.size() == 1 || lastSelectedObject < 1) return;
		sourceObjects.erase(sourceObjects.begin() + lastSelectedObject - 1);
		pickingShaders.erase(pickingShaders.begin() + lastSelectedObject - 1);
		UpdateShader();
		lastSelectedObject = -1;
	}
}

void InteractionManager::MoveSelectedObject(float xOffset, float yOffset, int windowWidth, int windowHeight, Camera& cam)
{
	if (LeftMouseReleased || selectedObject == 0) return;
	// fov is constant at 70d in core
	float dist = glm::distance(sourceObjects[selectedObject - 1]->GetPos(), cam.camPos);
	float PixelToWorld = 2 * dist * tan(glm::radians(35.0f)) / (float)windowHeight;

	glm::vec3 moveOffset = (xOffset * cam.camRight + yOffset * cam.camUp)* PixelToWorld;

	sourceObjects[selectedObject - 1]->AddPos(moveOffset * lockAxis);

	// update the starting seed positions
	if (computeManager->vistype == STREAM_LINES && computeManager->SeedingcomputeShaderID) {
		unsigned int xGroupNum = (computeManager->pointNum + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
		computeManager->SeedingcomputeShaderID->Compute(xGroupNum, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void InteractionManager::Resize(int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, FBOtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindRenderbuffer(GL_RENDERBUFFER, FBRO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void UpdateObjectSSBOval(ComputeManager &computeManager, ISourceObject &obj, int offset, int size, const void* val) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeManager.objectsSSBO);
	int index = obj.uniqueId * obj.GetStructSize() + obj.buffer_pos;
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, index + offset, size, val);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void InteractionManager::ImGuiWindow(float deltaTime){
	int objID = lastSelectedObject - 1;
	ImGui::Text(std::to_string(1 / deltaTime).c_str());
	if(objID >= 0 && ImGui::TreeNode("Selected Object")){

		ImGui::Text(("Selected ID: " + 
			sourceObjects[objID]->typeID + std::to_string(sourceObjects[objID]->uniqueId)).c_str());
		glm::vec3 pos =  sourceObjects[objID]->GetPos();
		if (ImGui::InputFloat3("Position", &pos.x))
			sourceObjects[objID]->MoveTo(pos);

		if (ImGui::InputFloat("Charge", &sourceObjects[objID]->charge))
		{
			UpdateObjectSSBOval(*computeManager, *sourceObjects[objID], sizeof(glm::vec3), sizeof(float),
				&sourceObjects[objID]->charge);
			UpdateShader();
		}
		// add radius as well, this is not good design since it doesn't consider if a new object is added with properties beyond
		// position, charge and now radius. however i'll just design that system when i need it, can't be bothered right now, i am tired:)
		if (sourceObjects[objID]->typeID == "InfiniteChargedCylinder" ||
			sourceObjects[objID]->typeID == "ChargedSphere" ||
			sourceObjects[objID]->typeID == "ChargedCircle") {
			auto* rObj = dynamic_cast<IHasRadius*>(sourceObjects[objID].get());
			if (ImGui::InputFloat("Radius", &(rObj->getRadius())));
			{
				// radius always after pos and charge
				UpdateObjectSSBOval(*computeManager, *sourceObjects[objID], sizeof(glm::vec3) + sizeof(float), sizeof(float),
					&rObj->getRadius());
			}
		}

		if (ImGui::InputInt("Seed Number", &sourceObjects[objID]->seedNum))
			UpdateShader();

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Visualisation Settings")) {
		const char* modes[] = {"3d grid", "stream lines"};
		if (ImGui::Combo("Vis Mode", (int*)&computeManager->vistype, modes, 2)) {

			renderer->vistype = computeManager->vistype;
			if (computeManager->vistype == GRID_3D) {
				computeManager->ConfigureGrid3D();
				renderer->ConfigureGrid3D(computeManager->gridSize, computeManager->gridGap);
			}

			else if (computeManager->vistype == STREAM_LINES) {
				computeManager->ConfigureStreamLines();
				renderer->ConfigureStreamLines(computeManager->stepNum, computeManager->pointNum);
			}
		}
		if (ImGui::Combo("Field", &selectedField, fields, sizeof(fields) / sizeof(fields[0]))) {

			computeManager->fieldType = fields[selectedField];
			UpdateShader();
		}

		// stream lines
		if (computeManager->vistype == visType::STREAM_LINES){
			ImGui::Checkbox("Dashed Stream Lines", &renderer->dashed);
			if (ImGui::InputFloat("TimeStep", &computeManager->streamLinesdeltaTime))
				UpdateShader();
			if (ImGui::InputInt("stepNum", &computeManager->stepNum)) {
				renderer->ConfigureStreamLines(computeManager->stepNum, computeManager->pointNum);
				UpdateShader();
			}
		}

		// grid
		if (computeManager->vistype == visType::GRID_3D) {
			if (ImGui::InputFloat3("gridSize", &computeManager->gridSize[0]) ||
				ImGui::InputFloat3("gridGap", &computeManager->gridGap[0])) {
				renderer->ConfigureGrid3D(computeManager->gridSize, computeManager->gridGap);
				UpdateShader();
			}
		}

		// particles
		if(ImGui::TreeNode("particles")) {
			ImGui::InputFloat3("Particles Num", &particlesNumv.x);
			ImGui::InputFloat3("Particles Gap", &particlesGapv.x);
			if (ImGui::SliderFloat("Particle Size", &particleSize, 0, 10)) {
				renderer->ParticlesShader->UseProgram();
				renderer->ParticlesShader->SetFloat("particleSize", particleSize);
			}
			if (ImGui::Button("Reset Particles")) 
				computeManager->InitParticles(particlesNumv, particlesGapv);

			if (ImGui::SmallButton("Delete Particles")) {
				computeManager->InitParticles(glm::vec3(0), glm::vec3(0));
				computeManager->particlesNum = 0;
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}

void InteractionManager::UpdateShader()
{
	computeManager->computeShaderID = std::make_unique<ComputeShader>(computeManager->GenerateComputeShaderSource()); // !!!! REGENERATE ENTIRE COMPUTE SHADER, mandatory:( !!!!
	renderer->positionBuffer = computeManager->positionBuffer;
	renderer->pointNum = computeManager->pointNum;
}

void InteractionManager::SetLeftMouseRelease(bool val)
{
	LeftMouseReleased = val;
	if (val == true) { 
		lockAxis = glm::vec3(1);
		selectedObject = 0;
	}
}