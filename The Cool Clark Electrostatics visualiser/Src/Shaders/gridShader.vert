#version 430 core

layout (binding = 0, std430) buffer positionBuffer{
	vec2 calculatedPos[];
};

uniform int gridWidth;
uniform int gridHeight;

void main(){
	int gridSize = gridWidth*gridHeight;
	int part = gl_VertexID % 2;
	int id = gl_InstanceID;
	float x = id%gridWidth;
	float y = id/gridWidth;

	vec2 gridPos = vec2(x/float(gridWidth), y/float(gridHeight));
	gridPos = (gridPos -.5)*2;

	if(part == 1){
		gl_Position = vec4(gridPos,0,1);
		return;
	}
	// not the best visualisation!
	float scale = .05;
	float r = length(calculatedPos[id]);
	vec2 pos;
	float max_val = 1.0;
	if(r > max_val) pos = normalize(calculatedPos[id])*max_val;
	else{
		pos = calculatedPos[id];
	}
	gl_Position = vec4(gridPos + pos*scale,0,1);
}