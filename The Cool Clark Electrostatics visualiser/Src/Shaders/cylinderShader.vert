#version 430

layout (location = 0) in vec3 aPos;
out vec3 Pos;
layout (std140, binding = 0) uniform Matrices{
	uniform mat4 view;
	uniform mat4 prespective;
};

uniform vec3 position;
uniform float radius;
void main(){
	vec3 aPosRadius = aPos;
	aPosRadius.xz *= radius;
	vec3 defPos = (position + aPosRadius);

	gl_Position = prespective * view * vec4(defPos, 1);
	Pos = aPos;
}