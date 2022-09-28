#include "Application.h"

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
		glm::mat2 Transform{ 1.0f };
		glm::vec2 Offset;
		alignas(16) glm::vec3 Color;
	};

	Application::Application()
	{
		LoadGameObjects();
		CreatePipelineLayout();
		RecreateSwapChain();
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

	void Application::LoadGameObjects()
	{
		std::vector<VEModel::Vertex> vertices = {};

		Sierpinski(vertices, 4, { 0.0f, -0.5f }, { 0.5f,  0.5f }, { -0.5f,  0.5f });
		
		auto model = std::make_shared<VEModel>(device, vertices);

		auto triangle = VEGameObject::CreateGameObject();

		triangle.m_Model							= model;
		triangle.m_Color							= { 0.1f, 0.8f, 0.1f };
		triangle.m_Transform2D.Translation.x		= 0.2f;
		triangle.m_Transform2D.Scale				= { 2.0f, 0.5f };
		triangle.m_Transform2D.Rotation				= 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

	void Application::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset					= 0;
		pushConstantRange.size						= sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= 0;
		pipelineLayoutInfo.pSetLayouts				= nullptr;
		pipelineLayoutInfo.pushConstantRangeCount	= 1;
		pipelineLayoutInfo.pPushConstantRanges		= &pushConstantRange;

		if (vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void Application::CreatePipeline()
	{
		assert(swapchain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig = {};

		VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.RenderPass					= swapchain->GetRenderPass();
		pipelineConfig.PipelineLayout				= pipelineLayout;

		pipeline = std::make_unique<VEPipeline>(device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void Application::RecreateSwapChain()
	{
		auto extent = window.GetExtent();

		while (extent.width == 0 || extent.height == 0)
		{
			extent = window.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.Device());

		if(swapchain == nullptr)
		{
			swapchain = std::make_unique<VESwapChain>(device, extent);
		}
		else
		{
			swapchain = std::make_unique<VESwapChain>(device, extent, std::move(swapchain));

			if (swapchain->ImageCount() != commandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}

		CreatePipeline();
	}

	void Application::CreateCommandBuffers()
	{
		commandBuffers.resize(swapchain->ImageCount());

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level							= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool					= device.GetCommandPool();
		allocInfo.commandBufferCount			= static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.Device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers.");
		}
	}

	void Application::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(device.Device(),
			device.GetCommandPool(),
			static_cast<float>(commandBuffers.size()),
			commandBuffers.data());

		commandBuffers.clear();
	}

	void Application::RecordCommandBuffer(uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo = {};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer.");
		}

		VkRenderPassBeginInfo renderPassInfo = {};

		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= swapchain->GetRenderPass();
		renderPassInfo.framebuffer				= swapchain->GetFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent		= swapchain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues = {};

		clearValues[0].color					= { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil				= { 1.0f, 0 };

		renderPassInfo.clearValueCount			= static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues				= clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};

		viewport.x								= 0.0f;
		viewport.y								= 0.0f;
		viewport.width							= static_cast<float>(swapchain->GetSwapChainExtent().width);
		viewport.height							= static_cast<float>(swapchain->GetSwapChainExtent().height);
		viewport.minDepth						= 0.0f;
		viewport.maxDepth						= 1.0f;

		VkRect2D scissor{ {0, 0}, swapchain->GetSwapChainExtent() };

		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		RenderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);

		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer.");
		}
	}
	 
	void Application::RenderGameObjects(VkCommandBuffer commandBuffer)
	{
		pipeline->Bind(commandBuffer);

		for (auto& obj : gameObjects)
		{
			obj.m_Transform2D.Rotation			= glm::mod(obj.m_Transform2D.Rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push = {};

			push.Offset							= obj.m_Transform2D.Translation;
			push.Color							= obj.m_Color;
			push.Transform						= obj.m_Transform2D.Mat2();

			vkCmdPushConstants(commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.m_Model->Bind(commandBuffer);
			obj.m_Model->Draw(commandBuffer);
		}
	}

	void Application::DrawFrame()
	{
		uint32_t imageIndex;
		auto result = swapchain->AcquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to aquire the next swap chain image");
		}

		RecordCommandBuffer(imageIndex);
		result = swapchain->SubmitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.WasWindowResized())
		{
			window.ResetWindowResizeFlag();
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to aquire the next swap chain image");
		}
	}

	void Application::Sierpinski(std::vector<VEModel::Vertex>& vertices,
		int depth,
		glm::vec2 top,
		glm::vec2 right,
		glm::vec2 left)
	{
		if (depth <= 0)
		{
			vertices.push_back({ top });
			vertices.push_back({ right });
			vertices.push_back({ left });
		
		}
		else
		{
			auto leftTop	= 0.5f * (left + top);
			auto rightTop	= 0.5f * (right + top);
			auto leftRight	= 0.5f * (left + right);
			Sierpinski(vertices, depth - 1, leftTop, leftRight, left);
			Sierpinski(vertices, depth - 1, rightTop, right, leftRight);
			Sierpinski(vertices, depth - 1, top, rightTop, leftTop);
			
		}
	}

}