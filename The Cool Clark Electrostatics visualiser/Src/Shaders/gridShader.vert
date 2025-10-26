#version 430 core

layout (binding = 0, std430) buffer positionBuffer{
	vec3 calculatedPos[];
};
layout (binding = 0, std140) uniform Matrices{
	mat4 view;
	mat4 prespective;
};

uniform int gridWidth;
uniform int gridHeight;
uniform int gridLength;

out float magnitude;

void main(){
	int gridSize = gridWidth*gridHeight*gridLength;
	int part = gl_VertexID % 2; // 1 = tail, 0 = head
	int id = gl_InstanceID;
	float x = id%gridWidth;
	float y = id/gridWidth;
	float z = 0; // TODO: make this work with z too, part II: return of the 
	vec3 E = calculatedPos[id];
	magnitude = length(E);
	vec3 gridPos = vec3(x/float(gridWidth), y/float(gridHeight), 0);
	gridPos = (gridPos -.5)*2;

	if(part == 1){
		gl_Position = prespective * view * vec4(gridPos.x, gridPos.y, 0 ,1);
		return;
	}

	// not the best visualisation!
	float scale = .05;
	float r = length(E);
	vec3 pos;
	float max_val = 1.0;
	if(r > max_val) pos = normalize(E)*max_val;
	else{
		pos = E;
	}
	vec4 apos = vec4(vec2(gridPos) + vec2(pos)*scale, 0,1);
	gl_Position = prespective * view * apos;
}