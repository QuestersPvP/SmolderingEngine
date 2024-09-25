#version 450       // Use GLSL 4.5

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texture;

layout (set = 0, binding = 1) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;
} uboViewProjection;

layout (push_constant) uniform PushModel
{
    mat4 model;
    int useTexture;
} pushModel;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexture;
layout (location = 2) out int useTexture;

void main() 
{
    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(position, 1.0);

    fragColor = color;
    fragTexture = texture;
    useTexture = pushModel.useTexture;
}