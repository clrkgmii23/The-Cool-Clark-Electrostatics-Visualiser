#version 430 core

out vec4 FragColor;
in float magnitude;
void main(){
	FragColor = vec4(magnitude/50, 0, 0, log(magnitude/3));
}