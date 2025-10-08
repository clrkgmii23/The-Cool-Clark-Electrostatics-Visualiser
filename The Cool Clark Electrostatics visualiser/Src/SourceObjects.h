#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

// a kind of "container class" so I can group all Source Objects into one vector
class ISourceObject {
public:
	virtual void Draw() = 0;
	virtual void ComputeElectricFieldContribution() = 0;
	virtual glm::vec3 GetPos() { return glm::vec3(0); };
	virtual void MoveTo(glm::vec3 trans) = 0;
	virtual ~ISourceObject() = default;
};

template<typename DERIVED>
class SourceObject : public ISourceObject {
public:
	Shader& shader;

	// every object needs a position and a charge
	glm::vec3 pos;
	double charge;


	SourceObject( glm::vec3 &pos, float charge, Shader& shader): charge(charge), shader(shader),
	pos(pos)
	{
		if (counter == 0) {
			static_cast<DERIVED*>(this)->initialSetUp();
		}
		counter++;
	}

	~SourceObject() {
		counter--;
		if (counter != 0) return;
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &EBO);
	}

	virtual void AfterSetUp(unsigned int verticesSize, float* vertices, unsigned int indicesSize, unsigned int* indices) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// VBO
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

		// EBO
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

		// VAO - currenty holdes x y z coords
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		indicesCount = indicesSize / sizeof(unsigned int);
	}

	// drawing aspect
	virtual void Draw() {


		glBindVertexArray(VAO);
		shader.UseProgram();
		shader.SetVec3("position", pos); // <-- TODO: REMOVE this, it's for testing
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);	
	}


	// computing aspect
	virtual void ComputeElectricFieldContribution() = 0; // for later

	// other stuff
	glm::vec3 GetPos() override { return pos;};

	void MoveTo(glm::vec3 trans) override {
		pos = trans;
	}

private:

	//
	static unsigned int VAO;
	static unsigned int VBO;
	static unsigned int EBO;
	static int indicesCount;

	static int counter;
};

// static stuff need to be declared outside of function for whatever reason
template<typename DERIVED>
unsigned int SourceObject<DERIVED>::VAO = 0;

template<typename DERIVED>
unsigned int SourceObject<DERIVED>::VBO = 0;

template<typename DERIVED>
unsigned int SourceObject<DERIVED>::EBO = 0;

template<typename DERIVED>
int SourceObject<DERIVED>::indicesCount = 0;

template<typename DERIVED>
int SourceObject<DERIVED>::counter= 0;


// now every object should add an initialSetUp, where it sets up the vertex and element buffer
// and then call AfterSetUp and it should be good to go

class pointCharge : public SourceObject<pointCharge> {

public:

	pointCharge(glm::vec3 pos, float charge, Shader& shader) 
		: SourceObject<pointCharge>(pos, charge, shader){}

	void initialSetUp() {

		shader.UseProgram(); 

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

	void ComputeElectricFieldContribution() override {
		
	}
};