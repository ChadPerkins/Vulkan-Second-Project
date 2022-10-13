#include "Application.h"
#include "InputController.h"
#include "Systems/SimpleRenderSystem.h"
#include "Systems/PointLightSystem.h"

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
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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
		
		PointLightSystem pointLightSystem(device, renderer.GetSwapChainRenderPass(), globalSetLayout->GetDescriptorSetLayout());

		VECamera camera = {};

		auto viewerObject = VEGameObject::CreateGameObject();
		viewerObject.m_Transform.Translation.z = -2.5f;
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
			camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
			
			if (auto commandBuffer = renderer.BeginFrame())
			{
				uint32_t frameIndex = renderer.GetFrameIndex();
				FrameInfo frameInfo = {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				// Update
				GlobalUbo ubo = {};
				ubo.ProjectionMatrix = camera.GetProjection();
				ubo.ViewMatrix =  camera.GetView();
				pointLightSystem.Update(frameInfo, ubo);
				uboBuffers[frameIndex]->WriteToBuffer(&ubo);
				uboBuffers[frameIndex]->Flush();

				// Render
				renderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(frameInfo);
				pointLightSystem.Render(frameInfo);
				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}
		}

		// Block the CPU until all GPU operations are completed
		vkDeviceWaitIdle(device.Device());
	}

	void Application::LoadGameObjects()
	{
		std::shared_ptr<VEModel> model			= VEModel::CreateModelFromFile(device, "Models/flat_vase.obj");

		auto flatVase		= VEGameObject::CreateGameObject();
		flatVase.m_Model						= model;
		flatVase.m_Transform.Translation		= { -0.5f, 0.5f, 0.0f };
		flatVase.m_Transform.Scale				= { 3.0f, 1.5f, 3.0f };

		gameObjects.emplace(flatVase.GetId(), std::move(flatVase));

		model									= VEModel::CreateModelFromFile(device, "Models/smooth_vase.obj");

		auto smoothVase		= VEGameObject::CreateGameObject();
		smoothVase.m_Model						= model;
		smoothVase.m_Transform.Translation		= { 0.5f, 0.5f, 0.0f };
		smoothVase.m_Transform.Scale			= { 3.0f, 1.5f, 3.0f };

		gameObjects.emplace(smoothVase.GetId(), std::move(smoothVase));

		model = VEModel::CreateModelFromFile(device, "Models/quad.obj");

		auto floor			= VEGameObject::CreateGameObject();
		floor.m_Model							= model;
		floor.m_Transform.Translation			= { 0.0f, 0.5f, 0.0f };
		floor.m_Transform.Scale					= { 3.0f, 1.0f, 3.0f };

		gameObjects.emplace(floor.GetId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{ 1.0f, 0.1f, 0.1f },
			{ 0.1f, 0.1f, 1.0f },
			{ 0.1f, 1.0f, 0.1f },
			{ 1.0f, 1.0f, 0.1f },
			{ 0.1f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f }
		};

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = VEGameObject::MakePointLight(0.2f);
			pointLight.m_Color = lightColors[i];

			// Create a circle and spread the lights evenly around the circle
			auto rotateLight = glm::rotate(glm::mat4(1.0f),
				i * glm::two_pi<float>() / lightColors.size(),
				{ 0.0f, -1.0f, 0.0f });

			pointLight.m_Transform.Translation = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));

			gameObjects.emplace(pointLight.GetId(), std::move(pointLight));
		}
	}	
}