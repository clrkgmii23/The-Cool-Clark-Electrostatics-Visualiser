#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 Pos;

uniform float aspectRatio;
uniform vec3 position;

void main(){
	gl_Position = vec4(vec2(aPos.x/aspectRatio, aPos.y)*.2, 0, 1) + vec4(position, 0);

	Pos = aPos;
}