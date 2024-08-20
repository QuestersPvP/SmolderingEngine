#version 450	// Use GLSL 4.5

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexture;
layout(location = 2) flat in int useTexture;

layout (set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;		// Final output color (must have location)

void main()
{
	
	// NOTE: if use texture = 1, use texture, IF 0 then just use fragment colors.
	if (useTexture == 1) 
	{
		outColor = texture(textureSampler, fragTexture);
	}
	else
	{
		outColor = vec4(fragColor, 1.0);
	}
}