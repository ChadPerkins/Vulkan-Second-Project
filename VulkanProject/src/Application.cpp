#include "Application.h"
#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace VulkanEngine {

	Application::Application()
	{
		LoadGameObjects();
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		SimpleRenderSystem simpleRenderSystem(device, renderer.GetSwapChainRenderPass());

		while (!window.Close())
		{
			glfwPollEvents();
			
			if (auto commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(commandBuffer, gameObjects);
				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
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
		
	void Application::Sierpinski(std::vector<VEModel::Vertex>& vertices,
		uint32_t depth,
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