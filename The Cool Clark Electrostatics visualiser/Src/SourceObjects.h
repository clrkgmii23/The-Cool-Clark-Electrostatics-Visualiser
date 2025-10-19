#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "fstream"
#include "utils.h"
// a kind of "container class" so I can group all Source Objects into one vector
class ISourceObject {
public:
	unsigned int buffer_pos = 0;

	virtual void Draw() = 0;
	virtual std::string ElectricFieldContribution() = 0;
	virtual void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) = 0;
	virtual glm::vec3 GetPos() { return glm::vec3(0);};
	virtual void MoveTo(glm::vec3 trans) = 0;
	virtual int GetStructSize() = 0;
	virtual void store(unsigned int buffer, int buf_pos, int own_pos, unsigned size, const void* data) = 0;
	std::string typeID;
	int uniqueId; 
	static int SSBObuffer;
	virtual ~ISourceObject() = default;
	
};

template<typename DERIVED, typename DERIVEDSTRUCT>
class SourceObject : public ISourceObject {
public:
	Shader& shader;
	// every object needs a position
	glm::vec3 pos;

	SourceObject( glm::vec3 &pos, Shader& shader): shader(shader),
	pos(pos)
	{
		if (counter == 0) {
			static_cast<DERIVED*>(static_cast<void*>(this))->initialSetUp();
		}
		counter++;
	}

	~SourceObject() {
		counter--;
		if (counter != 0) return;
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &EBO);
	}

	virtual void AfterSetUp(unsigned int verticesSize, float* vertices, unsigned int indicesSize, unsigned int* indices) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// VBO
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

		if(indices != nullptr) {
			// EBO
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);
			indicesCount = indicesSize / sizeof(unsigned int);
		}
		// VAO - currenty holdes x y z coords
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	// drawing aspect
	virtual void Draw() {
		glBindVertexArray(VAO);
		shader.UseProgram();
		shader.SetVec3("position", pos);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glm::vec3 GetPos() override { return pos; };


	void MoveTo(glm::vec3 trans) override {
		pos = trans;
		// send it to the buffer too
		if (SSBObuffer == -1) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBObuffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, buffer_pos, sizeof(glm::vec3), &pos);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	// computing aspect
	virtual std::string ElectricFieldContribution() { // get function to generate electric field in a compute shader

		// super hacky and jank way of getting the function definition from a file, but works
		std::ifstream file("Src/Shaders/SODefinitions.comp");
		if (!file.is_open()) {
			ErrorMessage("Could Not Open Source Object Definitions File");
		}
		std::string funcDef;
		std::string line;
		bool found = false;
		// function name is typeID + "ElectricField"
		std::string funcName = "vec3 " + typeID + "ElectricField";
		std::string structName = "struct " + typeID;
		while (std::getline(file, line)) {
			if (found) break;
			if (line.compare(0, funcName.length(), funcName) == 0) {
				funcDef += line + "\n";
				while (std::getline(file, line)) {
					funcDef += line + "\n";
					if (line == "}") {
						found = true;
						break;
						}
					}
			}
			else if (line.compare(0, structName.length(), structName) == 0) {
				// struct
				funcDef += line + "\n";
				while (std::getline(file, line)) {
					funcDef += line + "\n";
					if (line == "};") {
						funcDef += "\n";
						break;
					}
				}
			}
		}

		file.close();
		if (!found) ErrorMessage("Could Not Find Function Definition For " + funcName);

		funcDef += "\n\n";
		return funcDef; 
	}

	int GetStructSize() {
		return sizeof(DERIVEDSTRUCT);
	}

	void store(unsigned int buffer, int buf_pos, int own_pos, unsigned size, const void* data) override {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, buf_pos + own_pos * size, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
protected:

	static unsigned int VAO;
	static unsigned int VBO;
	static unsigned int EBO;
	static int indicesCount;

	static int counter;
};


// static stuff need to be declared outside of function for whatever reason
template<typename DERIVED, typename DERIVEDSTRUCT>
unsigned int SourceObject<DERIVED, DERIVEDSTRUCT>::VAO = 0;

template<typename DERIVED, typename DERIVEDSTRUCT>
unsigned int SourceObject<DERIVED, DERIVEDSTRUCT>::VBO = 0;

template<typename DERIVED, typename DERIVEDSTRUCT>
unsigned int SourceObject<DERIVED, DERIVEDSTRUCT>::EBO = 0;

template<typename DERIVED, typename DERIVEDSTRUCT>
int SourceObject<DERIVED, DERIVEDSTRUCT>::indicesCount = 0;

template<typename DERIVED, typename DERIVEDSTRUCT>
int SourceObject<DERIVED, DERIVEDSTRUCT>::counter = 0;

//  -------------------------------- define objects onwards --------------------------------  //

// now every object should just follow the structure of this point charge class


//-------------------------------------- POINT CHARGE --------------------------------------//

struct alignas(16) PointChargeStruct { // this will take 32 bytes
	glm::vec3 position;
	float charge;
};

class PointCharge : public SourceObject<PointCharge, PointChargeStruct> {
public:
	// properties
	float charge = 1.0f;
	PointCharge(glm::vec3 pos, float charge, Shader& shader)
		: SourceObject<PointCharge, PointChargeStruct>(pos, shader) {
		typeID = "PointCharge";
		this->charge = charge;
	}

	void initialSetUp() {
		// 4 points to make a square, fragment shader will take the rest and make it round
		float vertices[] = {
			 .5,  .5, 0,
			-.5,  .5, 0,
			 .5, -.5, 0,
			-.5, -.5, 0
		};

		unsigned int indices[] = {
			0, 1, 2,
			1, 3, 2
		};

		AfterSetUp(sizeof(vertices), vertices, sizeof(indices), indices);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {

		PointChargeStruct pointCharge = { pos, charge };
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &pointCharge);
	}
};

//--------------------------------------- INFINITE LINE ---------------------------------------//

struct alignas(16) InfiniteChargedLineStruct {
	glm::vec3 position;
	float charge;
};

class InfiniteChargedLine : public SourceObject<InfiniteChargedLine, InfiniteChargedLineStruct> {
public:
	// properties
	float charge = 1.0f;
	InfiniteChargedLine(glm::vec3 pos, float charge, Shader& shader)
		: SourceObject<InfiniteChargedLine, InfiniteChargedLineStruct>(pos, shader) {
		typeID = "InfiniteChargedLine";
		this->charge = charge;
	}

	void initialSetUp() {
		float vertices[] = {
			 0, 1,0,
			 0,-1,0
		};

		unsigned int indices[] = {
			0, 1
		};

		AfterSetUp(sizeof(vertices), vertices, sizeof(indices), indices);
	}

	void Draw() override {
		glBindVertexArray(VAO);
		shader.UseProgram();
		shader.SetVec3("position", pos);
		glDrawElements(GL_LINES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {

		InfiniteChargedLineStruct infiniteLineCharge = { pos, charge };
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &infiniteLineCharge);
	}
};

//-------------------------------------- CHARGED CIRCLE --------------------------------------//

struct alignas(16) ChargedCircleStruct {
	glm::vec3 position;
	float charge;
	float radius;
};
// TODO: instead of rendering it with lines, use the fragment shader to draw a circle from a quad, like what you did for point charges
class ChargedCircle : public SourceObject<ChargedCircle, ChargedCircleStruct> {
public:
	// properties
	float charge = 1.0;
	float radius = 0.5;
	ChargedCircle(glm::vec3 pos, float charge, float radius, Shader& shader)
		: SourceObject<ChargedCircle, ChargedCircleStruct>(pos, shader), radius(radius),
	charge(charge) {
		typeID = "ChargedCircle";
	}

	void initialSetUp() {
		std::vector<float> vertices;
		const float pi = 3.14159265359;
		// TODO: don't hard code 100 here
		const int N = 100;
		//this->radius = 0.5;
		for (int i = 0; i < 4*N + 2; i++)
		{
			float angle = (2 * pi) * i / N;
			vertices.push_back(this->pos.x + cos(angle));
			vertices.push_back(this->pos.y + sin(angle));
			vertices.push_back(this->pos.z);
		}


		AfterSetUp(vertices.size(), vertices.data(), 0, nullptr);
	}

	void Draw() override {
		glBindVertexArray(VAO);
		shader.UseProgram();
		const int N = 100;
		shader.SetVec3("position", pos);
		shader.SetFloat("radius", this->radius);
		glDrawArrays(GL_LINE_STRIP, 0, N+1);
		glBindVertexArray(0);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {

		ChargedCircleStruct chargedCircle = { pos, charge, radius};
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &chargedCircle);
	}
};