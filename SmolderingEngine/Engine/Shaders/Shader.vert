#version 450       // Use GLSL 4.5

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texture;

layout (set = 0, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;
} uboViewProjection;

//layout (set = 0, binding = 1) uniform UboModel
//{
//    mat4 model;
//} uboModel;

layout (push_constant) uniform PushModel
{
    mat4 model;
} pushModel;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexture;

layout (location = 2) out float useTexture;

void main() {
    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(position, 1.0);

    fragColor = color;
    fragTexture = texture;

    // Assume 0 if no texture is bound, 1 otherwise
    useTexture = (inTexCoord != vec2(0.0)) ? 1.0 : 0.0;
}