#pragma once
#include "VE_Device.h"
#include "VE_Model.h"
#include "VE_Pipeline.h"
#include "VE_SwapChain.h"
#include "VE_Window.h"

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
		void LoadModels();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void DrawFrame();

		void RecreateSwapChain();
		void RecordCommandBuffer(uint32_t imageIndex);

		// Recursive triangle effect
		void Sierpinski(std::vector<VEModel::Vertex>& vertices,
			int depth,
			glm::vec2 top,
			glm::vec2 right,
			glm::vec2 left);

	private:
		VEWindow window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
		VEDevice device{ window };
		std::unique_ptr<VESwapChain> swapchain;
		std::unique_ptr<VEPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<VEModel> model;
	};
}