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
		glm::mat4 Transform{ 1.0f };
		glm::mat4 NormalMatrix{ 1.0f };
	};

	SimpleRenderSystem::SimpleRenderSystem(VEDevice& device, VkRenderPass renderPass)
		: m_Device{device}
	{
		CreatePipelineLayout();
		CreatePipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.Device(), m_PipelineLayout, nullptr);
	}

	void SimpleRenderSystem::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT |
													  VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset					= 0;
		pushConstantRange.size						= sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= 0;
		pipelineLayoutInfo.pSetLayouts				= nullptr;
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

	void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VEGameObject>& gameObjects, const VECamera& camera)
	{
		m_Pipeline->Bind(commandBuffer);

		auto projectionView = camera.GetProjection() * camera.GetView();

		for (auto& obj : gameObjects)
		{
			SimplePushConstantData push = {};

			auto modelMatrix					= obj.m_Transform.Mat4();
			push.Transform							= projectionView * modelMatrix;
			push.NormalMatrix						= obj.m_Transform.NormalMatrix();

			vkCmdPushConstants(commandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.m_Model->Bind(commandBuffer);
			obj.m_Model->Draw(commandBuffer);
		}
	}
}