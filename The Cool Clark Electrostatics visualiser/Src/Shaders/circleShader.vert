#version 430 core

layout (location = 0) in vec3 aPos;
out vec3 Pos;

uniform vec3 position;
uniform float radius;

layout (std140, binding = 0) uniform Matrices{
    mat4 view;
    mat4 prespective;
};

void main()
{
    vec3 worldPos = aPos * radius + position;
    gl_Position = prespective * view * vec4(worldPos, 1.0);
    Pos = normalize(aPos);
}
