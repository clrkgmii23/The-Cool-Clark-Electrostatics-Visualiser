#version 430 core

layout(std430, binding = 2) buffer particlesBuffer{
	vec3 particleInfo[];
};

layout (std140, binding = 0) uniform Matrices{
	uniform mat4 view;
	uniform mat4 prespective;
};

flat out int instanceID;
uniform float particleSize;
void main(){
	int id = gl_InstanceID;
	vec3 pos = particleInfo[2*id];
	vec4 viewPos = view * vec4(pos, 1);
	gl_Position = prespective * viewPos;
	gl_PointSize = clamp(particleSize/-viewPos.z, .5, 200);
	//gl_PointSize = .7;

	instanceID = id;
}