#pragma once
#include "VE_Camera.h"

#include <vulkan/vulkan.h>

namespace VulkanEngine {

	struct FrameInfo
	{
		uint32_t FrameIndex;
		float FrameTime;
		VkCommandBuffer CommandBuffer;
		VECamera& Camera;
		VkDescriptorSet GlobalDescriptorSet;
	};
}