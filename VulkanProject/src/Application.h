#pragma once
#include "VE_Window.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "Vulkan Application";

namespace VulkanEngine {

	class Application
	{
	public:
		void Run();

	private:
		VEWindow window{WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE};
	};
}