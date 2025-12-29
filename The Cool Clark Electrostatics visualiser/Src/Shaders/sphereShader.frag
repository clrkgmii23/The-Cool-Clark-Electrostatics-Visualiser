#version 430 core

out vec4 FragColor;

in vec3 Pos;
void main(){
	// simple diffuse 
	vec3 col = vec3(0.92, .91, 0.81);
	vec3 lightDir = normalize(vec3(2, 3, 1));
	vec3 normal = Pos;
	vec3 difStr = max(dot(normal, lightDir), 0)*col;
	FragColor = vec4(difStr + col*0.1,1);
}