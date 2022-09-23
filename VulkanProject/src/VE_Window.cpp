#include "VE_Window.h"

#include <stdexcept>

namespace VulkanEngine {
	VEWindow::VEWindow(int width, int height, std::string title)
		: m_Width(width), m_Height(height), m_Title(title)
	{
		InitWindow();
	}
	
	VEWindow::~VEWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void VEWindow::InitWindow()
	{
		glfwInit();
		// Disable OpenGL functionality since Vulkan is being used
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Disable glfw from handling window resizing
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	}

	void VEWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a window surface.");
		}
	}
}