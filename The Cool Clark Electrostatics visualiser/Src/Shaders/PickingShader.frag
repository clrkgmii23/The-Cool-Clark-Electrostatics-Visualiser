#version 430 core

out vec4 FragColor;

uniform int id;


void main(){
	float red   = id & (255);
	float green = (id >> 8) & (255);
	float blue  = (id >> 16) & (255);
	FragColor = vec4(red/255.0, green/255.0, blue/255.0, 1);
}