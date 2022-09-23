#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace VulkanEngine {

	class VEWindow {
	public:
		VEWindow(int width, int height, std::string title);
		~VEWindow();

		// Delete the copy constructor and copy operator
		VEWindow(const VEWindow&) = delete;
		VEWindow& operator=(const VEWindow&) = delete;

		bool Close() { return glfwWindowShouldClose(m_Window); }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		void InitWindow();

	private:
		GLFWwindow* m_Window;
		std::string m_Title;
		const int m_Width;
		const int m_Height;

	};
}