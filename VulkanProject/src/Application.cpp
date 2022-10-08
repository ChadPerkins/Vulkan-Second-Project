#include "Application.h"
#include "InputController.h"
#include "SimpleRenderSystem.h"

#include "VE_Buffer.h"
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

	// Uniform buffer object
	struct GlobalUbo
	{
		alignas(16) glm::mat4 ProjectionView{ 1.0f };
		alignas(16) glm::vec3 LightDirection = glm::normalize(glm::vec3(1.0f, -3.0f, -1.0f));

	};

	Application::Application()
	{
		globalPool = VEDescriptorPool::Builder(device)
			.SetMaxSets(VESwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VESwapChain::MAX_FRAMES_IN_FLIGHT)
			.Build();


		LoadGameObjects();
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		std::vector<std::unique_ptr<VEBuffer>> uboBuffers(VESwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VEBuffer>(
				device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->Map();
		}

		auto globalSetLayout = VEDescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		std::vector<VkDescriptorSet> globalDescriptorSets(VESwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->DescriptorInfo();

			VEDescriptorWriter(*globalSetLayout, *globalPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem(device, renderer.GetSwapChainRenderPass(), globalSetLayout->GetDescriptorSetLayout());
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
				uint32_t frameIndex = renderer.GetFrameIndex();
				FrameInfo frameInfo = {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				// Update
				GlobalUbo ubo = {};
				ubo.ProjectionView = camera.GetProjection() * camera.GetView();
				uboBuffers[frameIndex]->WriteToBuffer(&ubo);
				uboBuffers[frameIndex]->Flush();

				// Render
				renderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(frameInfo, gameObjects);
				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
		}

		// Block the CPU until all GPU operations are completed
		vkDeviceWaitIdle(device.Device());
	}

	void Application::LoadGameObjects()
	{
		std::shared_ptr<VEModel> model		= VEModel::CreateModelFromFile(device, "Models/flat_vase.obj");

		auto flatVase	= VEGameObject::CreateGameObject();
		flatVase.m_Model						= model;
		flatVase.m_Transform.Translation		= { -0.5f, 0.5f, 2.5f };
		flatVase.m_Transform.Scale			= { 3.0f, 1.5f, 3.0f };

		gameObjects.push_back(std::move(flatVase));

		model								 = VEModel::CreateModelFromFile(device, "Models/smooth_vase.obj");

		auto smoothVase	= VEGameObject::CreateGameObject();
		smoothVase.m_Model						= model;
		smoothVase.m_Transform.Translation		= { 0.5f, 0.5f, 2.5f };
		smoothVase.m_Transform.Scale			= { 3.0f, 1.5f, 3.0f };

		gameObjects.push_back(std::move(smoothVase));
	}
	
}