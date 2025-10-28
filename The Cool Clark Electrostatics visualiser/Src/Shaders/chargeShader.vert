#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 Pos;

uniform vec3 position;

layout (std140, binding = 0) uniform Matrices{
	uniform mat4 view;
	uniform mat4 prespective;
};

void main(){
	mat3 newView = mat3(view);
	mat3 inView = transpose(newView);
	vec3 billboardPos = inView * (aPos*0.1);
	vec4 worldPos = vec4(billboardPos + position, 1.0);
	gl_Position = prespective * view * worldPos;

	Pos = aPos;
}