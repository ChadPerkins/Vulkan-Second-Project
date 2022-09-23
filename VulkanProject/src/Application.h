#pragma once
#include "VE_Device.h"
#include "VE_Pipeline.h"
#include "VE_Window.h"

const uint32_t WINDOW_WIDTH = 1280;
const uint32_t WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "Vulkan Application";

namespace VulkanEngine {

	class Application
	{
	public:
		void Run();

	private:
		VEWindow window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
		VEDevice device{ window };
		VEPipeline pipeline{ device,
			"Shaders/Simple_Shader.vert.spv",
			"Shaders/Simple_Shader.frag.spv",
			VEPipeline::DefaultPipelineConfigInfo(WINDOW_WIDTH, WINDOW_HEIGHT) };
	};
}