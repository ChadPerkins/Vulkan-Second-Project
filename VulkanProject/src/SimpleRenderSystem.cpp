#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace VulkanEngine {

	struct SimplePushConstantData
	{
		glm::mat4 ModelMatrix{ 1.0f };
		glm::mat4 NormalMatrix{ 1.0f };
	};

	SimpleRenderSystem::SimpleRenderSystem(VEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: m_Device{device}
	{
		CreatePipelineLayout(globalSetLayout);
		CreatePipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.Device(), m_PipelineLayout, nullptr);
	}

	void SimpleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT |
													  VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset					= 0;
		pushConstantRange.size						= sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts				= descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount	= 1;
		pipelineLayoutInfo.pPushConstantRanges		= &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig = {};

		VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.RenderPass					= renderPass;
		pipelineConfig.PipelineLayout				= m_PipelineLayout;

		m_Pipeline = std::make_unique<VEPipeline>(m_Device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::RenderGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects)
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

		for (auto& obj : gameObjects)
		{
			SimplePushConstantData push = {};

			push.ModelMatrix						= obj.m_Transform.Mat4();
			push.NormalMatrix						= obj.m_Transform.NormalMatrix();

			vkCmdPushConstants(frameInfo.CommandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.m_Model->Bind(frameInfo.CommandBuffer);
			obj.m_Model->Draw(frameInfo.CommandBuffer);
		}
	}
}