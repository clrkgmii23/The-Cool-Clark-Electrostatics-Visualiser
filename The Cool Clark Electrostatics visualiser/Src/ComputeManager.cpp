#include <glad/glad.h>
#include "ComputeManager.h"
#include <format>


int ISourceObject::SSBObuffer = -1; // c++ quirk

ComputeManager::ComputeManager(std::vector<std::unique_ptr<ISourceObject>>& sourceObjects, const char* computeShaderPath, glm::vec3 gridSize, glm::vec3 gridGap):
	sourceObjects(sourceObjects), gridSize(gridSize), gridGap(gridGap)
{
	computeShaderID = std::make_unique<ComputeShader>(GenerateComputeShaderSource());
	// creating SSBO for grid positions
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	unsigned int gridSizeN = gridSize.x* gridSize.y* gridSize.z;
	// std430 apparently requires padding to vec4, so multiplying by 4 rather than 3 for the vec3 array
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridSizeN * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeManager::ComputeContributions()
{
	unsigned int xGroupNum = (gridSize.x + X_INVOCATION_NUM - 1) / X_INVOCATION_NUM;
	unsigned int yGroupNum = (gridSize.y + Y_INVOCATION_NUM - 1) / Y_INVOCATION_NUM;
	unsigned int zGroupNum = (gridSize.z + Z_INVOCATION_NUM - 1) / Z_INVOCATION_NUM;
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
		"float k = 1; \n\n"
		"float e_0 = 1; \n\n"
		"float PI = 3.14159265359; \n\n";
	
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
	computeShaderSource += "layout(std430, binding = 1)	buffer INpositionBuffer{\n";
	for (auto& pair : typeInfos) {
		computeShaderSource += "\t" + pair.first + " " + pair.first + "s[" + std::to_string(pair.second.count) + "];\n";
		pair.second.bufStartPos = prevBufPos;
		prevBufPos = pair.second.bufStartPos + pair.second.count * pair.second.structSize;
	}
	computeShaderSource += "};\n\n\n";


	// main function
	computeShaderSource += "void main(){\n"
		"\tvec3 gridSize = gl_NumWorkGroups.xyz * gl_WorkGroupSize.xyz;\n"
		"\tvec3 gridPos = vec3(gl_GlobalInvocationID);\n"
		"\gridPos = (gridPos - ((vec3(gridSize)-1.0)/2.0))*vec3(" + std::to_string(gridGap.x) + "," + std::to_string(gridGap.y) +
		", " + std::to_string(gridGap.z) + ");\n"

		"\tuint id = uint(gl_GlobalInvocationID.x + (gl_GlobalInvocationID.y * gridSize.x) + gl_GlobalInvocationID.z * gridSize.x * gridSize.y);\n"
		"\tvec3 E = vec3(0);\n";
	int SSBOobjectsize = 0;
	// per type operations here
	for (const auto& pair : typeInfos) {
		computeShaderSource += "\tfor(int i = 0; i < " + std::to_string(pair.second.count) + "; i++){\n"
			"\t\tE += " + pair.first + "ElectricField(" + pair.first + "s[i], gridPos);\n\t}\n\n";
	}

	computeShaderSource += "\tcalculatedPos[id] = E;\n}";
	
	// make position buffer for source objects. this is here because we need bufsize
	glGenBuffers(1, &objectsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufsize, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectsSSBO);

	SendSOjectsPos();

	Info(computeShaderSource);

	return computeShaderSource;
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