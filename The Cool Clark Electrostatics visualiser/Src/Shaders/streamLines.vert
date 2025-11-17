#version 430 core

layout (binding = 0, std430) buffer positionBuffer{
	vec3 calculatedPos[];
};
layout (binding = 0, std140) uniform Matrices{
	mat4 view;
	mat4 prespective;
};

uniform int pointNum;
uniform int stepNum;

out float magnitude;

void main(){
	int id = gl_InstanceID; // which line
	int point = gl_VertexID; // which point in the line
	vec3 pos = calculatedPos[id*stepNum + point];
	vec3 next_pos = calculatedPos[id*stepNum + point + 1];
	magnitude = distance(pos, next_pos);
	gl_Position = prespective * view * vec4(pos, 1);
}