#version 430 core

out vec4 FragColor;
in vec3 Pos;
uniform float charge;
void main(){
float r = length(Pos);
    if(r > 0.5) discard;  // hard circular cutoff

	float d = smoothstep(.5, .47,length(Pos));
	vec3 col = mix(vec3(.4,.4,1), vec3(1,.4,.4), 2*charge - 0.5*charge);
	FragColor = vec4(col.x*d, col.y*d, col.z*d, d);
}