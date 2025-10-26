#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 Pos;

uniform vec3 position;

layout (std140, binding = 0) uniform Matrices{
	uniform mat4 view;
	uniform mat4 prespective;
};

void main(){
	gl_Position = prespective * view *  (vec4(vec2(aPos.x, aPos.y)*.2, 0, 1) + vec4(position, 0));

	Pos = aPos;
}