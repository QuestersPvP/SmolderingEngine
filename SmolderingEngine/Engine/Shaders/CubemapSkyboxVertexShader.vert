#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragTexCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    mat4 viewMatrix = mat4(mat3(ubo.view)); // Remove translation
    gl_Position = ubo.proj * viewMatrix * vec4(inPosition, 1.0);
    fragTexCoord = inPosition;
}

//#version 450
//
//layout(location = 0) in vec3 inPosition;
//
//layout(location = 0) out vec3 vTexCoords;
//
//layout(set = 0, binding = 0) uniform UniformBufferObject {
//    mat4 projection;
//    mat4 view;
//} ubo;
//
//void main() {
//    // Pass the vertex position as texture coordinates
//    vTexCoords = inPosition;
//
//    // Remove translation from the view matrix
//    mat4 viewNoTranslation = mat4(mat3(ubo.view));
//
//    // Apply projection and view matrices
//    gl_Position = ubo.projection * viewNoTranslation * vec4(inPosition, 1.0);
//    // Set w component to 1.0 to prevent clipping at near plane
//    gl_Position.w = gl_Position.z;
//}
