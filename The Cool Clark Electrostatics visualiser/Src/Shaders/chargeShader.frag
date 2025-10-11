#version 430 core

out vec4 FragColor;
in vec3 Pos;
void main(){
	float d = smoothstep(.5, .48,length(Pos));
	FragColor = vec4(d, d, d, d);
}