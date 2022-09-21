#include "Application.h"

namespace VulkanEngine {
	
	void Application::Run()
	{
		while (!window.Close())
		{
			glfwPollEvents();
		}
	}
}