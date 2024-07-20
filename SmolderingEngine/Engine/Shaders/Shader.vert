#version 450       // Use GLSL 4.5

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;

layout (binding = 0) uniform ModelViewProjection
{
    mat4 projection;
    mat4 view;
    mat4 model;
} modelViewProjection;

layout (location = 0) out vec3 fragCol;

void main() {
    gl_Position = modelViewProjection.projection * modelViewProjection.view * modelViewProjection.model * vec4(pos, 1.0);

    fragCol = col;
}