#pragma once
#include "VE_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace VulkanEngine {

	class VEModel
	{
	public:
		struct Vertex
		{
			glm::vec3 Position{};
			glm::vec3 Color{};
			glm::vec3 Normal{};
			glm::vec2 UV{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return Position == other.Position &&
					Color == other.Color &&
					Normal == other.Normal &&
					UV == other.UV;
			}
		};

		struct Builder
		{
			std::vector<Vertex> Vertices{};
			std::vector<uint32_t> Indices{};

			void LoadModel(const std::string& filepath);
		};

		VEModel(VEDevice& device, const VEModel::Builder& builder);
		~VEModel();

		// Delete the copy constructor and copy operator
		VEModel(const VEModel&) = delete;
		VEModel& operator=(const VEModel&) = delete;

		static std::unique_ptr<VEModel> CreateModelFromFile(VEDevice& device, const std::string& filepath);

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);

	private:
		VEDevice& m_Device;
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		uint32_t m_VertexCount;

		bool m_HasIndexBuffer = false;

		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;
		uint32_t m_IndexCount;
	};
}