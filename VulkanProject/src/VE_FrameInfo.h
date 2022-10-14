#pragma once
#include "VE_Camera.h"
#include "VE_GameObject.h"

#include <vulkan/vulkan.h>

namespace VulkanEngine {

#define MAX_LIGHTS 10

	struct PointLight
	{
		glm::vec4 Position{}; // Ignore w
		glm::vec4 Color{};    // W is the intensity
	};

	// Uniform buffer object
	struct GlobalUbo
	{
		glm::mat4 ProjectionMatrix{ 1.0f };
		glm::mat4 ViewMatrix{ 1.0f };
		glm::mat4 InverseViewMatrix{ 1.0f };
		glm::vec4 AmbientLightColor{ 1.0f, 1.0f, 1.0f, 0.1f }; // W is the intensity
		
		PointLight PointLights[MAX_LIGHTS];
		int NumLights;
	};

	struct FrameInfo
	{
		uint32_t FrameIndex;
		float FrameTime;
		VkCommandBuffer CommandBuffer;
		VECamera& Camera;
		VkDescriptorSet GlobalDescriptorSet;
		VEGameObject::Map& GameObjects;
	};
}