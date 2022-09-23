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
		VkExtent2D GetExtent() { return { m_Width, m_Height }; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		void InitWindow();

	private:
		GLFWwindow* m_Window;
		std::string m_Title;
		const uint32_t m_Width;
		const uint32_t m_Height;

	};
}