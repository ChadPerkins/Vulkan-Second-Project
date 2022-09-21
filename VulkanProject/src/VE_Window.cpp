#include "VE_Window.h"

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
}