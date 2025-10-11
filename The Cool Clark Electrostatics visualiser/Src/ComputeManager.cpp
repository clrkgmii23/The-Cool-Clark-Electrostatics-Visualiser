#include <glad/glad.h>
#include "ComputeManager.h"
#include <format>

ComputeManager::ComputeManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, const char* computeShaderPath, unsigned int gridWidth, unsigned int gridHeight, unsigned int gridLength):
	sourceObjects(sourceObjects), gridWidth(gridWidth), gridHeight(gridHeight), gridLength(gridLength)
{
	computeShaderID = std::make_unique<ComputeShader>(GenerateComputeShaderSource());
	// creating SSBO for grid positions
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	unsigned int gridSize = gridWidth*gridHeight* gridLength;
	// std430 apparently requires padding to vec4, so multiplying by 4 rather than 3 for the vec3 array
	// TODO: test this, isn't 430 supposed to get rid of padding?
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridSize * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeManager::ComputeContributions()
{
	unsigned int xGroupNum = (gridWidth  + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
	unsigned int yGroupNum = (gridHeight + Y_INVOCATION_NUM - 1) / Y_INVOCATION_NUM;
	unsigned int zGroupNum = (gridLength + Z_INVOCATION_NUM - 1) / Z_INVOCATION_NUM;
	computeShaderID->Compute(xGroupNum, yGroupNum, zGroupNum, GL_SHADER_STORAGE_BARRIER_BIT);
}

// TODO: currently, this is set up to generate a compute shader that will return
// the electric field in device coordinates; normalized. in the future make it more customizable 
std::string ComputeManager::GenerateComputeShaderSource()
{
	
	std::string computeShaderSource = "#version 430 core\n";
	computeShaderSource +=
		"layout (local_size_x = " + std::to_string(X_INVOCATION_NUM) +
		", local_size_y = " + std::to_string(Y_INVOCATION_NUM) +
		", local_size_z = " + std::to_string(Z_INVOCATION_NUM) +
		") in;\n" +
		"layout(binding = 0, std430) buffer positionBuffer {\n"
		"	vec3 calculatedPos[];\n};\n\n" +
		"float k = 1; \n\n";
	
	int bufsize = 0;

	for(auto& SObject : sourceObjects)
	{
		bufsize += SObject->GetStructSize();
		// per type in list
		if (typeInfos.find(SObject->typeID) == typeInfos.end()) {
			computeShaderSource += SObject->ElectricFieldContribution();
			typeInfos[SObject->typeID].count = 1;
			typeInfos[SObject->typeID].structSize = SObject->GetStructSize();
			SObject->uniqueId = 0;
			continue;
		}
		typeInfos[SObject->typeID].count++;
		SObject->uniqueId = typeInfos[SObject->typeID].count - 1;
	}

	int prevBufPos = 0;
	computeShaderSource += "layout(std430, binding = 1)	buffer INpositionBuffer{\n\t";
	for (auto& pair : typeInfos) {
		computeShaderSource += pair.first + " " + pair.first  + "s[];\n";
		pair.second.bufStartPos = prevBufPos;
		prevBufPos = pair.second.bufStartPos + pair.second.count * pair.second.structSize;
	}
	computeShaderSource += "};\n\n\n";


	// main function
	computeShaderSource += "void main(){\n"
		"\tvec3 gridSize = gl_NumWorkGroups.xyz * gl_WorkGroupSize.xyz;\n"
		"\tvec3 gridPos = vec3(gl_GlobalInvocationID) / (gridSize);\n"
		"\tgridPos = (gridPos - .5) * 2;\n"
		"\tuint id = uint(gl_GlobalInvocationID.x + (gl_GlobalInvocationID.y * gridSize.x) + gl_GlobalInvocationID.z * gridSize.x * gridSize.y);\n"
		"\tvec3 E = vec3(0);\n";
	int SSBOobjectsize = 0;
	// per type operations here
	for (const auto& pair : typeInfos) {
		computeShaderSource += "\tfor(int i = 0; i < " + std::to_string(pair.second.count) + "; i++){\n"
			"\t\tE += " + pair.first + "ElectricField(" + pair.first + "s[i], gridPos);\n\t}\n\n";
	}

	computeShaderSource += "\tcalculatedPos[id] = E;\n}";
	//Info(computeShaderSource);
	
	// make position buffer for source objects
	glGenBuffers(1, &objectsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufsize, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectsSSBO);
	// send in the positions

	for (auto& SObject : sourceObjects) {
		SObject->StoreInBuffer(objectsSSBO, typeInfos[SObject->typeID].bufStartPos, SObject->uniqueId);
	}

	Info(computeShaderSource);
	return computeShaderSource;
}