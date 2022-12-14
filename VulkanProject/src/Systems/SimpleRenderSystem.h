#pragma once
#include "VE_Camera.h"
#include "VE_Device.h"
#include "VE_FrameInfo.h"
#include "VE_GameObject.h"
#include "VE_Pipeline.h"

#include <memory>
#include <vector>


namespace VulkanEngine {

	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		// Delete the copy constructor and copy operator
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void RenderGameObjects(FrameInfo& frameInfo);

	private:
		void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void CreatePipeline(VkRenderPass renderPass);

	private:
		VEDevice& m_Device;
		std::unique_ptr<VEPipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;
	};
}