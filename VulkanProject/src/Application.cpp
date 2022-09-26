#include "Application.h"

#include <array>
#include <stdexcept>

namespace VulkanEngine {
	Application::Application()
	{
		LoadModels();
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
			DrawFrame();
		}

		// Block the CPU until all GPU operations are completed
		vkDeviceWaitIdle(device.Device());
	}

	void Application::LoadModels()
	{
		std::vector<VEModel::Vertex> vertices = {};

		Sierpinski(vertices, 5, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });

		model = std::make_unique<VEModel>(device, vertices);
	}

	void Application::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount						= 0;
		pipelineLayoutInfo.pSetLayouts							= nullptr;
		pipelineLayoutInfo.pushConstantRangeCount				= 0;
		pipelineLayoutInfo.pPushConstantRanges					= nullptr;

		if (vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void Application::CreatePipeline()
	{
		auto pipelineConfig				= VEPipeline::DefaultPipelineConfigInfo(swapchain.Width(), swapchain.Height());
		pipelineConfig.RenderPass								= swapchain.GetRenderPass();
		pipelineConfig.PipelineLayout							= pipelineLayout;

		pipeline = std::make_unique<VEPipeline>(device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void Application::CreateCommandBuffers()
	{
		commandBuffers.resize(swapchain.ImageCount());

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType											= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level											= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool									= device.GetCommandPool();
		allocInfo.commandBufferCount							= static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.Device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers.");
		}

		for (int i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};

			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin recording command buffer.");
			}
			
			VkRenderPassBeginInfo renderPassInfo = {};

			renderPassInfo.sType								= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass							= swapchain.GetRenderPass();
			renderPassInfo.framebuffer							= swapchain.GetFrameBuffer(i);

			renderPassInfo.renderArea.offset					= { 0, 0 };
			renderPassInfo.renderArea.extent					= swapchain.GetSwapChainExtent();

			std::array<VkClearValue, 2> clearValues = {};

			clearValues[0].color								= { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil							= { 1.0f, 0 };

			renderPassInfo.clearValueCount						= static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues							= clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			pipeline->Bind(commandBuffers[i]);

			model->Bind(commandBuffers[i]);
			model->Draw(commandBuffers[i]);
			
			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer.");
			}
		}
	}
	 
	void Application::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = swapchain.AcquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to aquire the next swap chain image");
		}

		result = swapchain.SubmitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to aquire the next swap chain image");
		}
	}

	void Application::Sierpinski(
		std::vector<VEModel::Vertex>& vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({ top });
			vertices.push_back({ right });
			vertices.push_back({ left });
		}
		else {
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			Sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			Sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			Sierpinski(vertices, depth - 1, leftTop, rightTop, top);
		}
	}
}