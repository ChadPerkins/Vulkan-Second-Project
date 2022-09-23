#pragma once
#include "VE_Device.h"
#include "VE_Pipeline.h"
#include "VE_Window.h"
#include "VE_SwapChain.h"

#include <memory>
#include <vector>

const uint32_t WINDOW_WIDTH = 1280;
const uint32_t WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "Vulkan Application";

namespace VulkanEngine {

	class Application
	{
	public:
		Application();
		~Application();

		// Delete the copy constructor and copy operator
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void Run();

	private:
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();

	private:
		VEWindow window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
		VEDevice device{ window };
		VESwapChain swapchain{ device, window.GetExtent() };
		std::unique_ptr<VEPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
	};
}