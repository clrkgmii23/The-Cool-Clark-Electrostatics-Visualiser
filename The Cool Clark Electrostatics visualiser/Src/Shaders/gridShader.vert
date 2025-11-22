#version 430 core

layout (binding = 0, std430) buffer positionBuffer{
	vec3 calculatedPos[];
};
layout (binding = 0, std140) uniform Matrices{
	mat4 view;
	mat4 prespective;
};

uniform ivec3 gridSize;

uniform vec3 gridGap;

out float magnitude;

void main(){
	int gridSizeN = int( gridSize.x*gridSize.y*gridSize.z);
	int part = gl_VertexID % 2; // 1 = tail, 0 = head
	int id = gl_InstanceID;
	float x = id%gridSize.x;
	float y = (id/gridSize.x)%(gridSize.y);
	float z = id/(gridSize.x*gridSize.y); 


	vec3 E = calculatedPos[id];
	magnitude = length(E);
	vec3 gridPos = vec3(x, y, z);
	gridPos = (gridPos - (vec3(gridSize) - 1.0)*0.5f) * gridGap;

	if(part == 1){
		gl_Position = prespective * view * vec4(gridPos,1);
		return;
	}

	// not the best visualisation!
	float scale = .05;
	vec3 pos;
	float max_val = 1.0;
	if(magnitude > max_val) pos = normalize(E)*max_val;
	else{
		pos = E;
	}
	vec4 apos = vec4(gridPos + pos*scale,1);
	gl_Position = prespective * view * apos;
}