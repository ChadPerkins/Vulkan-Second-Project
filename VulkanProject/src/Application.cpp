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

	void Application::LoadGameObjects()
	{
		std::shared_ptr<VEModel> model	= VEModel::CreateModelFromFile(device, "Models/smooth_vase.obj");

		auto gameObj = VEGameObject::CreateGameObject();
		gameObj.m_Model					= model;
		gameObj.m_Transform.Translation	= { 0.0f, 0.0f, 2.5f };
		gameObj.m_Transform.Scale			= glm::vec3(3.0f);

		gameObjects.push_back(std::move(gameObj));
	}
	
}