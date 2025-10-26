#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 Pos;

uniform vec3 position;

layout (std140, binding = 0) uniform Matrices{
	uniform mat4 view;
	uniform mat4 prespective;
};

void main(){
	gl_Position = prespective * view * vec4(aPos + position, 1);

	Pos = aPos;
}