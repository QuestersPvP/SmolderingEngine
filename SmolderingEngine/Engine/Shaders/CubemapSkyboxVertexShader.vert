#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragTexCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

void main() {
    mat4 viewMatrix = mat4(mat3(ubo.view)); // Remove translation
    gl_Position = ubo.proj * viewMatrix * vec4(inPosition, 1.0);
    fragTexCoord = inPosition;
}