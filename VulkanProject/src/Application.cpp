#include "Application.h"
#include "InputController.h"
#include "SimpleRenderSystem.h"

#include "VE_Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <chrono>
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

		VECamera camera = {};

		auto viewerObject = VEGameObject::CreateGameObject();

		InputController cameraController = {};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!window.Close())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.MoveInPlaneXZ(window.GetWindow(), frameTime, viewerObject);
			camera.SetViewYXZ(viewerObject.m_Transform.Translation, viewerObject.m_Transform.Rotation);

			float aspect = renderer.GetAspectRatio();
			camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);
			
			if (auto commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(commandBuffer, gameObjects, camera);
				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
		}

		// Block the CPU until all GPU operations are completed
		vkDeviceWaitIdle(device.Device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<VEModel> CreateCubeModel(VEDevice& device, glm::vec3 offset)
	{
		VEModel::Builder modelBuilder = {};

		modelBuilder.Vertices = {

			// left face (white)
			{{ -0.5f, -0.5f, -0.5f }, { 0.9f, 0.9f, 0.9f }},
			{{ -0.5f,  0.5f,  0.5f }, { 0.9f, 0.9f, 0.9f }},
			{{ -0.5f, -0.5f,  0.5f }, { 0.9f, 0.9f, 0.9f }},
			{{ -0.5f,  0.5f, -0.5f }, { 0.9f, 0.9f, 0.9f }},

	 		//  right face (yellow)
			{{  0.5f, -0.5f, -0.5f }, { 0.8f, 0.8f, 0.1f }},
			{{  0.5f,  0.5f,  0.5f }, { 0.8f, 0.8f, 0.1f }},
			{{  0.5f, -0.5f,  0.5f }, { 0.8f, 0.8f, 0.1f }},
			{{  0.5f,  0.5f, -0.5f }, { 0.8f, 0.8f, 0.1f }},

	 		//  top face (orange, remember y axis points down)
			{{ -0.5f, -0.5f, -0.5f }, { 0.9f, 0.6f, 0.1f }},
			{{  0.5f, -0.5f,  0.5f }, { 0.9f, 0.6f, 0.1f }},
			{{ -0.5f, -0.5f,  0.5f }, { 0.9f, 0.6f, 0.1f }},
			{{  0.5f, -0.5f, -0.5f }, { 0.9f, 0.6f, 0.1f }},

	 		//  bottom face (red)
			{{ -0.5f,  0.5f, -0.5f }, { 0.8f, 0.1f, 0.1f }},
			{{  0.5f,  0.5f,  0.5f }, { 0.8f, 0.1f, 0.1f }},
			{{ -0.5f,  0.5f,  0.5f }, { 0.8f, 0.1f, 0.1f }},
			{{  0.5f,  0.5f, -0.5f }, { 0.8f, 0.1f, 0.1f }},

	 		//  nose face (blue)
			{{ -0.5f, -0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f }},
			{{  0.5f,  0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f }},
			{{ -0.5f,  0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f }},
			{{  0.5f, -0.5f,  0.5f }, { 0.1f, 0.1f, 0.8f }},

	 		//  tail face (green)
			{{ -0.5f, -0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f }},
			{{  0.5f,  0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f }},
			{{ -0.5f,  0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f }},
			{{  0.5f, -0.5f, -0.5f }, { 0.1f, 0.8f, 0.1f }}

		};

		for (auto& v : modelBuilder.Vertices)
		{
			v.position += offset;
		}

		modelBuilder.Indices = {
			0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
			12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21
		};


		return std::make_unique<VEModel>(device, modelBuilder);
	}

	void Application::LoadGameObjects()
	{
		std::shared_ptr<VEModel> model	= CreateCubeModel(device, { 0.0f,0.0f, 0.0f });

		auto cube	= VEGameObject::CreateGameObject();
		cube.m_Model					= model;
		cube.m_Transform.Translation	= { 0.0f, 0.0f, 2.5f };
		cube.m_Transform.Scale			= { 0.5f, 0.5f, 0.5f };

		gameObjects.push_back(std::move(cube));
	}
	
}