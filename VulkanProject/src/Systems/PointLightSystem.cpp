#include "PointLightSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace VulkanEngine {

	PointLightSystem::PointLightSystem(VEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: m_Device{device}
	{
		CreatePipelineLayout(globalSetLayout);
		CreatePipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(m_Device.Device(), m_PipelineLayout, nullptr);
	}

	void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		/*VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT |
													  VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset					= 0;
		pushConstantRange.size						= sizeof(SimplePushConstantData);*/

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts				= descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount	= 0;
		pipelineLayoutInfo.pPushConstantRanges		= nullptr;

		if (vkCreatePipelineLayout(m_Device.Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void PointLightSystem::CreatePipeline(VkRenderPass renderPass)
	{
		assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig = {};

		VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.BindingDescriptions.clear();
		pipelineConfig.AttributeDescriptions.clear();

		pipelineConfig.RenderPass					= renderPass;
		pipelineConfig.PipelineLayout				= m_PipelineLayout;

		m_Pipeline = std::make_unique<VEPipeline>(m_Device,
			"Shaders/Point_Light.vert.spv",
			"Shaders/Point_Light.frag.spv",
			pipelineConfig);
	}

	void PointLightSystem::Render(FrameInfo& frameInfo)
	{
		m_Pipeline->Bind(frameInfo.CommandBuffer);

		vkCmdBindDescriptorSets(frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			0,
			1,
			&frameInfo.GlobalDescriptorSet,
			0,
			nullptr);

		vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
	}
}