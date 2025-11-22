#version 430 core

out vec4 FragColor;
layout(std430, binding = 2) buffer particlesBuffer{
	vec3 particleInfo[];
};


flat in int instanceID;

void main(){
	int id = instanceID;

	FragColor = vec4(particleInfo[2*id+1], 1);
}