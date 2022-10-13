#version 450
layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldSpacePos;
layout (location = 2) in vec3 fragNormalWorldSpace;

layout (location = 0) out vec4 outColor;

struct PointLight
{
		vec4 position; // Ignore w
		vec4 color;    // W is the intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor; // W is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorldSpace);

	for (int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];

		vec3 directionToLight = light.position.xyz - fragWorldSpacePos;		// Ignore the worldSpacePosition.w component
		float attenuation = 1.0 / dot(directionToLight, directionToLight);  // Distance squared

		float cosAngleIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);

		vec3 intensity = light.color.xyz * light.color.w * attenuation; 

		diffuseLight += intensity * cosAngleIncidence;
	}

	outColor = vec4(diffuseLight * fragColor, 1.0);
}