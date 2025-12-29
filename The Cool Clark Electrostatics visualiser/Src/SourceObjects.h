#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "fstream"
#include "utils.h"
// a kind of "container class" so I can group all Source Objects into one vector
class ISourceObject {
public:
	float charge = 1.0;
	int seedNum = 0;
	virtual void Draw(bool useOwnShader = true) = 0;
	virtual std::string FieldContribution(std::string funcName) = 0;
	virtual void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) = 0;
	virtual glm::vec3 GetPos() { return glm::vec3(0); };
	virtual void MoveTo(glm::vec3 trans) = 0;
	virtual void AddPos(glm::vec3 trans) = 0;
	virtual int GetStructSize() = 0;
	virtual Shader* GetShader() = 0;
	virtual void store(unsigned int buffer, int buf_pos, int own_pos, unsigned size, const void* data) = 0;
	unsigned int buffer_pos = 0;
	std::string typeID;
	int uniqueId = 0;
	static int SSBObuffer;
	virtual ~ISourceObject() = default;
};

template<typename DERIVED, typename DERIVEDSTRUCT>
class SourceObject : public ISourceObject {
public:
	// every object needs a position
	glm::vec3 pos;
	Shader& shader;

	SourceObject(glm::vec3& pos, Shader& shader) : shader(shader),
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

		if (indices != nullptr) {
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
	virtual void Draw(bool useOwnShader = true) {
		glBindVertexArray(VAO);
		if (useOwnShader)
			shader.UseProgram();
		shader.SetVec3("position", pos);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glm::vec3 GetPos() override { return pos; };


	void MoveTo(glm::vec3 trans) override {
		pos = trans;
		sendToBuffer();
	}

	void AddPos(glm::vec3 trans) override {
		pos += trans;
		sendToBuffer();
	}

	void sendToBuffer() {
		// update position in buffer 
		if (SSBObuffer == -1) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBObuffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, buffer_pos, sizeof(glm::vec3), &pos);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	// computing aspect
	virtual std::string FieldContribution(std::string funcName) {
		// this gets fieldCalc function + struct
		std::ifstream file("Src/Shaders/SODefinitions.comp");
		if (!file.is_open()) {
			ErrorMessage("Could Not Open Source Object Definitions File");
		}
		std::string funcDef;
		std::string line;
		bool found = false;
		// function name example: typeID + "ElectricField"
		funcName = "vec3 " + typeID + funcName;
		std::string structName = "struct " + typeID;
		while (std::getline(file, line)) {
			if (found) break; // break from the entire loop
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
			// include struct
			else if (line.compare(0, structName.length(), structName) == 0) {
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

	Shader* ISourceObject::GetShader() override {
		return &shader;
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

	void Draw(bool useOwnShader = true) override {
		glBindVertexArray(VAO);
		if (useOwnShader)
			shader.UseProgram();
		shader.SetVec3("position", pos);
		shader.SetFloat("charge", charge);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
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
	InfiniteChargedLine(glm::vec3 pos, float charge, Shader& shader)
		: SourceObject<InfiniteChargedLine, InfiniteChargedLineStruct>(pos, shader) {
		typeID = "InfiniteChargedLine";
		this->charge = charge;
	}

	void initialSetUp() {
		float vertices[] = {
			 0, 100,0,
			 0,-100,0
		};

		unsigned int indices[] = {
			0, 1
		};

		AfterSetUp(sizeof(vertices), vertices, sizeof(indices), indices);
	}

	void Draw(bool useOwnShader = true) override {
		glBindVertexArray(VAO);
		if (useOwnShader)
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

struct IHasRadius {
	float radius = 0.3;
	virtual float& getRadius() = 0;
};


//-------------------------------------- CHARGED CIRCLE --------------------------------------//

struct alignas(16) ChargedCircleStruct {
	glm::vec3 position;
	float charge;
	float radius;
};
// TODO: instead of rendering it with lines, use the fragment shader to draw a circle from a quad, like what you did for point charges
class ChargedCircle : public SourceObject<ChargedCircle, ChargedCircleStruct>, public IHasRadius {
public:
	// properties
	float radius = .1;
	int N = 25;
	ChargedCircle(glm::vec3 pos, float charge, float radius, Shader& shader)
		: SourceObject<ChargedCircle, ChargedCircleStruct>(pos, shader), radius(radius) {
		typeID = "ChargedCircle";
		this->charge = charge;
	}

	float& getRadius() override{
		return radius;
	}

	void initialSetUp() {
		N = 50; // resolution of the circle, can't get it out of here:(
		std::vector<float> vertices;
		const float pi = 3.14159265359;
		// TODO: don't hard code this
		for (int i = 0; i <= N; i++)
		{
			float angle = ((2 * pi) * i )/ N;
			vertices.push_back(cos(angle));
			vertices.push_back(sin(angle));
			vertices.push_back(0);
		}
		AfterSetUp(vertices.size()*sizeof(float), vertices.data(), 0, nullptr);
	}

	void Draw(bool useOwnShader = true) override {
		N = 50;
		glBindVertexArray(VAO);
		if (useOwnShader)
			shader.UseProgram();
		shader.SetVec3("position", pos);
		shader.SetFloat("radius", this->radius);
		glDrawArrays(GL_LINE_STRIP, 0, N+1);
		glBindVertexArray(0);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {
		ChargedCircleStruct chargedCircle = { pos, charge, radius };
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &chargedCircle);
	}
};


//--------------------------------- Infinite Charged Cylinder ---------------------------------//

struct alignas(16) InfiniteChargedCylinderStruct{ // this will take 32 bytes
	glm::vec3 position;
	float radius;
	float charge;
};

class InfiniteChargedCylinder : public SourceObject<InfiniteChargedCylinder, InfiniteChargedCylinderStruct>, public IHasRadius {
public:
	// properties
	float radius = .5;
	int N = 25;
	InfiniteChargedCylinder(glm::vec3 pos, float charge, float radius, Shader& shader)
		: SourceObject<InfiniteChargedCylinder, InfiniteChargedCylinderStruct>(pos, shader), radius(radius) {
		typeID = "InfiniteChargedCylinder";
	}

	float& getRadius() override {
		return radius;
	}

	void initialSetUp() {
		N = 50;

		std::vector<float> vertices;
		const float pi = 3.14159265359;
		// confusing code that i just worked out in a notebook so my future self can't debug these cryptic lines
		float h = 200; // height of our cylinder, temp before making it really infinite somehow
		// first circle
		for (int i = 0; i < N; i++)
		{
			float angle = ((2 * pi) * i) / N;
			vertices.push_back(cos(angle));
			vertices.push_back(h);
			vertices.push_back(sin(angle));
		}

		//second circle
		for (int i = 0; i < N; i++)
		{
			float angle = ((2 * pi) * i) / N;
			vertices.push_back(cos(angle));
			vertices.push_back(-h);
			vertices.push_back(sin(angle));
		}
		// indexing
		std::vector<unsigned int> indices;

		// caps, why would an infinite cylinder have caps?
		/*for (size_t i = 0; i < N-1; i++)
		{

			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(0);

			indices.push_back(i + N);
			indices.push_back(i + N+1);
			indices.push_back(N);

		}*/

		for (size_t i = 0; i < N; i++) {
			indices.push_back(i);
			indices.push_back((i +1)%N);
			indices.push_back((i + 1) % N + N);

			indices.push_back(i);
			indices.push_back(i + N);
			indices.push_back((i + 1)%N + N);
		}
		
		AfterSetUp(vertices.size() * sizeof(float), vertices.data(), indices.size()*sizeof(float), indices.data());
	}
	void Draw(bool useOwnShader = true) override{
		glBindVertexArray(VAO);
		if (useOwnShader)
			shader.UseProgram();
		shader.SetVec3("position", pos);
		shader.SetFloat("radius", this->radius);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {

		InfiniteChargedCylinderStruct cylinder = { pos, radius, charge };
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &cylinder);
	}
};

//-------------------------------------- CHARGED SPHERE --------------------------------------//

struct alignas(16) ChargedSphereStruct {
	glm::vec3 position;
	float charge;
	float radius;
};

class ChargedSphere : public SourceObject<ChargedSphere, ChargedSphereStruct>, public IHasRadius {
public:
	// properties
	float radius = .1;
	int N1 = 5;
	int N2 = 5;
	ChargedSphere(glm::vec3 pos, float charge, float radius, Shader& shader)
		: SourceObject<ChargedSphere, ChargedSphereStruct>(pos, shader), radius(radius) {
		typeID = "ChargedSphere";
		this->charge = charge;
	}

	float& getRadius() override {
		return radius;
	}

	void initialSetUp() {
		N1 = 50;
		N2 = 50;
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		const float pi = 3.14159265359;
		// spherical coords
		// x = rsinphi costheta
		// y = rsinphi sintheta
		// z = rcosphi
		
		for (int theta = 0; theta < N1; theta++)
		{
			float angleTheta = ((2 * pi) * theta) / N1;
			for (int phi = 0; phi < N2; phi++)
			{
				// vertices
				float anglePhi = (pi * phi) / (N2 - 1);


				float x = sin(anglePhi) * cos(angleTheta);
				float y = sin(anglePhi) * sin(angleTheta);
				float z = cos(anglePhi);

				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				// indices
				if (phi >= N2-1) continue;

				unsigned int current = theta*N2 + phi;
				unsigned int next = ((theta+1)%N1) *N2 + phi;


				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current+1);

				indices.push_back(current+1);
				indices.push_back(next);
				indices.push_back(next+1);
			}
		}

		AfterSetUp(vertices.size() * sizeof(float), vertices.data(), indices.size() * sizeof(unsigned int), indices.data());
	}

	void Draw(bool useOwnShader = true) override {
		glBindVertexArray(VAO);
		if (useOwnShader)
			shader.UseProgram();
		shader.SetVec3("position", pos);
		shader.SetFloat("radius", this->radius);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void StoreInBuffer(unsigned int buffer, int buf_pos, int own_pos) override {
		ChargedSphereStruct chargedCircle = { pos, charge, radius };
		int size = GetStructSize();

		this->store(buffer, buf_pos, own_pos, size, &chargedCircle);
	}
};