#version 450

layout (location = 0) in vec3 fragOffset;
layout (location = 0) out vec4 outColor;

 struct PointLight
{
		vec4 position; // Ignore w
		vec4 color;    // W is the intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 inverseViewMatrix;
	vec4 ambientLightColor; // W is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

void main()
{
	float dis = sqrt(dot(fragOffset, fragOffset));

	if (dis >= 1.0)
	{
		discard;
	}

	outColor = vec4(push.color.xyz, 1.0);
}