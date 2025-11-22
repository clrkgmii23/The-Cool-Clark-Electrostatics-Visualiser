#include <glad/glad.h>
#include "ComputeManager.h"
#include <format>

int ISourceObject::SSBObuffer = -1; // c++ quirk

ComputeManager::ComputeManager(visType vistype, std::vector<std::unique_ptr<ISourceObject>>& sourceObjects) :
	vistype(vistype), sourceObjects(sourceObjects)
{
}

void ComputeManager::ConfigureGrid3D(glm::vec3 _gridSize = glm::vec3(10), glm::vec3 _gridGap = glm::vec3(0.5)) {
	if (vistype != GRID_3D) ErrorMessage("Trying To Configure Wrong VISTYPE");
	gridSize = _gridSize;
	gridGap = _gridGap;

	computeShaderID = std::make_unique<ComputeShader>(GenerateComputeShaderSource());
}

void ComputeManager::ConfigureStreamLines(int _stepNum, float _streamLinesdeltaTime) {
	if (vistype != STREAM_LINES) ErrorMessage("Trying To Configure Wrong VISTYPE");
	stepNum = _stepNum;
	streamLinesdeltaTime = _streamLinesdeltaTime;
	computeShaderID = std::make_unique<ComputeShader>(GenerateComputeShaderSource());
}

void ComputeManager::Compute()
{
	if (vistype == GRID_3D) {
		unsigned int xGroupNum = (gridSize.x + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
		unsigned int yGroupNum = (gridSize.y + Y_INVOCATION_NUM - 1) / Y_INVOCATION_NUM;
		unsigned int zGroupNum = (gridSize.z + Z_INVOCATION_NUM - 1) / Z_INVOCATION_NUM;
		computeShaderID->Compute(xGroupNum, yGroupNum, zGroupNum, GL_SHADER_STORAGE_BARRIER_BIT);
	}
	else if (vistype == STREAM_LINES) {
		unsigned int xGroupNum = (pointNum + 64 - 1) / 64;
		computeShaderID->Compute(xGroupNum, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT);
	}
}


std::string ComputeManager::GenerateComputeShaderSource()
{
	// init, so we can regenerate any time (will change this so it doesn't create new ones everytime)
	if (positionBuffer > 0) {
		glDeleteBuffers(1, &positionBuffer);
	}
	if (objectsSSBO > 0) {
		glDeleteBuffers(1, &objectsSSBO);
	}

	typeInfos.clear();
	this->pointNum = 0; // crucial

	//
	std::string computeShaderSource = "#version 430 core\n";
	if (vistype == GRID_3D) {
		computeShaderSource += "layout (local_size_x = " + std::to_string(X_INVOCATION_NUM) +
			", local_size_y = " + std::to_string(Y_INVOCATION_NUM) +
			", local_size_z = " + std::to_string(Z_INVOCATION_NUM) +
			") in;\n";
	}
	else if (vistype == STREAM_LINES) {
		for (size_t i = 0; i < sourceObjects.size(); i++)
		{
			if (sourceObjects[i]->charge > 0)
				this->pointNum += sourceObjects[i]->seedNum;
		}

		computeShaderSource += "layout (local_size_x = 64"
			", local_size_y = 1"
			", local_size_z = 1"
			") in;\n";
	}

	computeShaderSource += "layout(binding = 0, std430) buffer positionBuffer {\n"
		"\tvec3 calculatedPos[];\n};\n\n"
		"float k = 1; \n" // change as your heart's content
		"float e_0 = 1; \n"
		"float PI = 3.14159265359;\n"
		"float PHI = 1.61803398874;\n";

	int bufsize = 0;
	// loop on every object type, and store information about it in typeInfos
	for (auto& SObject : sourceObjects)
	{
		bufsize += SObject->GetStructSize();
		if (typeInfos.find(SObject->typeID) == typeInfos.end()) {
			//if(vistype != STREAM_LINES) // TODO: this shader doesn't need the field functions, maybe seperate field functions and structs
			computeShaderSource += SObject->FieldContribution("ElectricField");
			typeInfos[SObject->typeID].count = 1;
			typeInfos[SObject->typeID].structSize = SObject->GetStructSize();
			SObject->uniqueId = 0;
			continue;
		}
		typeInfos[SObject->typeID].count++;
		SObject->uniqueId = typeInfos[SObject->typeID].count - 1;
	}

	int prevBufPos = 0;
	computeShaderSource += "layout(std430, binding = 1)	buffer INpositionBuffer{\n";
	for (auto& pair : typeInfos) {
		computeShaderSource += "\t" + pair.first + " " + pair.first + "s[" + std::to_string(pair.second.count) + "];\n";
		pair.second.bufStartPos = prevBufPos;
		prevBufPos = pair.second.bufStartPos + pair.second.count * pair.second.structSize;
	}
	computeShaderSource += "};\n\n\n";

	std::string fieldType = "ElectricField"; // TODO: you already know what to do
	computeShaderSource +=
		"\nvec3 contribution(vec3 pos){\n"
		"\tvec3 res = vec3(0);\n";
	for (const auto& pair : typeInfos) {
		computeShaderSource +=
			"\tfor(int j = 0; j < " + std::to_string(pair.second.count) + "; j++){\n"
			"\tres += " + pair.first + fieldType + "(" + pair.first + "s[j], pos);\n\t}\n\n";
	}
	computeShaderSource += "\nreturn res;\n}\n";

	// make position buffer for source objects. this is here because we need bufsize
	glGenBuffers(1, &objectsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufsize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectsSSBO);

	SendSOjectsPos();
	CreateParticleShader(computeShaderSource);
	// choose an appropriate main function
	switch (vistype) {
	case GRID_3D:
		grid3dSource(computeShaderSource);
		break;
	case STREAM_LINES:
		StreamLinesSource(computeShaderSource);
		break;
	default:
		ErrorMessage("Invalid Visualisation Type In Compute Shader");
		break;
	}

	//Info(computeShaderSource);

	return computeShaderSource;
}

void ComputeManager::grid3dSource(std::string& computeShaderSource) {

	unsigned int gridSizeN = gridSize.x * gridSize.y * gridSize.z;
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	// std430 apparently requires padding to vec4, so multiplying by 4 rather than 3 for the vec3 array
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridSizeN * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	computeShaderSource += "void main(){\n"
		"\tvec3 gridSize = gl_NumWorkGroups.xyz * gl_WorkGroupSize.xyz;\n"
		"\tvec3 gridPos = vec3(gl_GlobalInvocationID);\n"
		"\tgridPos = (gridPos - ((vec3(gridSize)-1.0)/2.0))*vec3(" + std::to_string(gridGap.x) + "," + std::to_string(gridGap.y) +
		", " + std::to_string(gridGap.z) + ");\n"

		"\tuint id = uint(gl_GlobalInvocationID.x + (gl_GlobalInvocationID.y * gridSize.x) + gl_GlobalInvocationID.z * gridSize.x * gridSize.y);\n"
		"\tvec3 E = contribution(gridPos);\n";

	computeShaderSource += "\tcalculatedPos[id] = E;\n}";
	//Info(computeShaderSource);
}

// TODO: actual good todo, don't really like this function, maybe find another way that doesn't read from the top everytime and close the file.
std::string GetPartFromFile(const char* filePath, const std::string startStr, const std::string endStr = "}") {
	// super hacky and jank way of getting the function definition from a file, but works
	std::ifstream file(filePath);
	if (!file.is_open()) {
		ErrorMessage("Could Not Open Source Object Definitions File");
	}
	std::string funcDef;
	std::string line;
	bool found = false;
	// function name is typeID + "ElectricField"
	while (std::getline(file, line)) {
		if (found) break; // break from the entire loop
		if (line.compare(0, startStr.length(), startStr) == 0) {
			funcDef += line + "\n";
			while (std::getline(file, line)) {
				funcDef += line + "\n";
				if (line == endStr) {
					found = true;
					break;
				}
			}
		}
	}
	file.close();
	return funcDef;
}

// TODO: maybe make seeds more costumizable?
void ComputeManager::StreamLinesSource(std::string& computeShaderSource) {

	// we will use a seperate compute shader to get the seeds for our streamlines. since it has a bunch of "if statement", which can cause 
	// problems as they increase in a shader. so we're only gonna calculate the seeds once in one shader, and update them every frame in our main shader.

	std::string seedingComputeShaderSource = computeShaderSource;
	seedingComputeShaderSource += "int stepAmount = " + std::to_string(stepNum) + ";\n";
	// include the seeding functions
	for (auto pair : typeInfos)
	{
		seedingComputeShaderSource += GetPartFromFile("Src/Shaders/SODefinitions.comp",
			"void " + pair.first + "DistributePoints");
	}

	seedingComputeShaderSource += "void main(){\n"
		"\tint i = int(gl_GlobalInvocationID.x);\n";

	computeShaderSource += "int stepAmount = " + std::to_string(stepNum) + ";\n";
	computeShaderSource += "float deltaTime = " + std::to_string(streamLinesdeltaTime) + ";\n"; // TODO: not constant

	int n_0 = 0;
	std::unordered_map<std::string, int> objectInfo;
	for (size_t i = 0; i < sourceObjects.size(); i++)
	{
		if (sourceObjects[i]->charge > 0) {
			std::string nameID = sourceObjects[i]->typeID;
			int n_1 = n_0 + sourceObjects[i]->seedNum;
			seedingComputeShaderSource += "\tif( " + std::to_string(n_0) + " <= i && i < " + std::to_string(n_1) + "){\n"
				"\t\t" + nameID + "DistributePoints(" + nameID + "s[" + std::to_string(objectInfo[nameID]++) + "], i, i - " + std::to_string(n_0) + ", " + std::to_string(n_1 - n_0) + "); \n\t }\n";
			n_0 = n_1;
		}
	}

	seedingComputeShaderSource += "}";

	//Info(seedingComputeShaderSource);

	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, stepNum * pointNum * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	SeedingcomputeShaderID = std::make_unique<ComputeShader>(seedingComputeShaderSource);
	unsigned int xGroupNum = (pointNum + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
	SeedingcomputeShaderID->Compute(xGroupNum, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT);
	// back to our original shader
	computeShaderSource +=
		"\nvoid main(){\n"
		"\tint x = int(gl_GlobalInvocationID.x);\n"
		"\tfor(int i = 0; i < stepAmount-1; i++){\n"
		"\t\tvec3 currentPos = calculatedPos[x*stepAmount + i];\n"

		// RK4 looks bad
		/*"\t\tvec3 K1 = contribution(currentPos);\n"
		"\t\tvec3 K2 = contribution(currentPos + K1*deltaTime*0.5);\n"
		"\t\tvec3 K3 = contribution(currentPos + K2*deltaTime*0.5);\n"
		"\t\tvec3 K4 = contribution(currentPos + K3*deltaTime);\n"

		"\t\tvec3 E = (1.0f/6.0f) * (K1 + 2*K2 + 2*K3 + K4);\n"*/
		// euler (only direction) looks good
		"\t\tvec3 E = normalize(contribution(currentPos));\n"

		"\t\tvec3 nextPos = currentPos + clamp(E, vec3(-10), vec3(10))*deltaTime;\n"
		"\t\tcalculatedPos[x*stepAmount + i + 1] = nextPos;\n";

	computeShaderSource += "\n\t}\n}";
}

// particles

void ComputeManager::InitParticles(glm::vec3 particlesNums, glm::vec3 particlesGap)
{
	particlesNum = particlesNums.x * particlesNums.y * particlesNums.z;
	// this buffer will go like this [x,y,z, Vx, Vy, Vz, ...] and repeats
	glGenBuffers(1, &particlesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNum * 2 * 4* sizeof(float), nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particlesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	ComputeShader initComp("Src/Shaders/initParticles.comp");
	initComp.UseProgram();
	initComp.SetIVec3("particleNum", particlesNums);
	initComp.SetVec3("particlesGap", particlesGap);

	int xGroupNum = (particlesNum + 64 - 1) / 64;
	initComp.Compute(xGroupNum, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT);
}

//this function relies on typeinfos!
void ComputeManager::CreateParticleShader(std::string computeShaderSource)
{
	computeShaderSource += "float deltaTime = " + std::to_string(streamLinesdeltaTime) + ";\n";

	computeShaderSource += 
		"layout(std430, binding = 2) buffer	particlesBuffer{\n\tvec3 particleInfo[];\n};\n\n"
		"void main(){\n"
		"\tint id = int(gl_GlobalInvocationID.x);\n"
		"\tvec3 currentPos = particleInfo[2*id];\n"
		"\tvec3 currentVel= particleInfo[2*id + 1];\n"
		"\tparticleInfo[2*id + 1] = currentVel + contribution(currentPos)*deltaTime;\n"
		"\tparticleInfo[2*id] = particleInfo[2*id] + particleInfo[2*id+1]*deltaTime;\n}";
	// maybe clamp vel?
	particleShaderID = std::make_unique<ComputeShader>(computeShaderSource);
	//Info(computeShaderSource);
}

void ComputeManager::UpdateParticles()
{
	int xGroupNum = (particlesNum + 64 - 1) / 64;
	particleShaderID->Compute(xGroupNum,1,1, GL_SHADER_STORAGE_BARRIER_BIT);
}


void ComputeManager::SendSOjectsPos() {
	// send in the positions and set buffer pos
	for (auto& SObject : sourceObjects) {
		SObject->buffer_pos = typeInfos[SObject->typeID].bufStartPos +
			+SObject->uniqueId
			* typeInfos[SObject->typeID].structSize;
		SObject->StoreInBuffer(objectsSSBO, typeInfos[SObject->typeID].bufStartPos, SObject->uniqueId);
	}

	ISourceObject::SSBObuffer = objectsSSBO;
}