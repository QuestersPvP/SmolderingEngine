#version 450

layout(location = 0) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;

void main() {
    outColor = texture(skyboxSampler, fragTexCoord);
}

//#version 450
//
//layout(location = 0) in vec3 vTexCoords;
//
//layout(location = 0) out vec4 outColor;
//
//layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;
//
//void main() {
//    outColor = texture(skyboxSampler, vTexCoords);
//}
