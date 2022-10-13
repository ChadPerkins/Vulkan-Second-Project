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

	struct PointLightPushConstants
	{
		glm::vec4 Position{};
		glm::vec4 Color{};
		float Radius;
	};

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
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT |
													  VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset					= 0;
		pushConstantRange.size						= sizeof(PointLightPushConstants);

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

	void PointLightSystem::Update(FrameInfo& frameInfo, GlobalUbo& ubo)
	{
		// Create a circle and spread the lights evenly around the circle
		auto rotateLight = glm::rotate(glm::mat4(1.0f),
			frameInfo.FrameTime,
			{ 0.0f, -1.0f, 0.0f });

		int lightIndex = 0;

		for (auto& kv : frameInfo.GameObjects)
		{
			auto& obj = kv.second;

			if (obj.m_PointLight == nullptr)
			{
				continue;
			}

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum number of lights permitted.");

			// Update the light's position
			obj.m_Transform.Translation = glm::vec3(rotateLight * glm::vec4(obj.m_Transform.Translation, 1.0f));

			// Copy the light to the ubo
			ubo.PointLights[lightIndex].Position	= glm::vec4(obj.m_Transform.Translation, 1.0f);
			ubo.PointLights[lightIndex].Color		= glm::vec4(obj.m_Color, obj.m_PointLight->LightIntensity);

			lightIndex += 1;
		}

		ubo.NumLights = lightIndex;
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

		for (auto& kv : frameInfo.GameObjects)
		{
			auto& obj = kv.second;

			if (obj.m_PointLight == nullptr)
			{
				continue;
			}

			PointLightPushConstants push = {};

			push.Position = glm::vec4(obj.m_Transform.Translation, 1.0f);
			push.Color = glm::vec4(obj.m_Color, obj.m_PointLight->LightIntensity);
			push.Radius = obj.m_Transform.Scale.x;

			vkCmdPushConstants(frameInfo.CommandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);

			vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
		}
	}
}