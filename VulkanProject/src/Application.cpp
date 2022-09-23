#include "Application.h"

#include <stdexcept>

namespace VulkanEngine {
	Application::Application()
	{
		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffers();
	}

	Application::~Application()
	{
		vkDestroyPipelineLayout(device.Device(), pipelineLayout, nullptr);
	}

	void Application::Run()
	{
		while (!window.Close())
		{
			glfwPollEvents();
		}
	}

	void Application::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void Application::CreatePipeline()
	{
		auto pipelineConfig = VEPipeline::DefaultPipelineConfigInfo(swapchain.Width(), swapchain.Height());
		pipelineConfig.RenderPass = swapchain.GetRenderPass();
		pipelineConfig.PipelineLayout = pipelineLayout;

		pipeline = std::make_unique<VEPipeline>(device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void Application::CreateCommandBuffers()
	{
	}

	void Application::DrawFrame()
	{
	}
}