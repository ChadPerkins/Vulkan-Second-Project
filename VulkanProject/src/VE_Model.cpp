#include "VE_Model.h"
#include "VE_Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>

namespace std {
	template <>
	struct hash<VulkanEngine::VEModel::Vertex>
	{
		size_t operator()(VulkanEngine::VEModel::Vertex const& vertex) const
		{
			size_t seed = 0;
			VulkanEngine::HashCombine(seed, vertex.Position, vertex.Color, vertex.Normal, vertex.UV);
			return seed;
		}
	};
}

namespace VulkanEngine {

	VEModel::VEModel(VEDevice& device, const VEModel::Builder& builder)
		: m_Device{device}
	{
		CreateVertexBuffers(builder.Vertices);
		CreateIndexBuffers(builder.Indices);
	}

	VEModel::~VEModel()
	{
		vkDestroyBuffer(m_Device.Device(), m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device.Device(), m_VertexBufferMemory, nullptr);

		if (m_HasIndexBuffer)
		{
			vkDestroyBuffer(m_Device.Device(), m_IndexBuffer, nullptr);
			vkFreeMemory(m_Device.Device(), m_IndexBufferMemory, nullptr);

		}
	}

	std::unique_ptr<VEModel> VEModel::CreateModelFromFile(VEDevice& device, const std::string& filepath)
	{
		Builder builder = {};
		builder.LoadModel(filepath);

		return std::make_unique<VEModel>(device, builder);
	}

	void VEModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_VertexCount >= 3 && "Vertex count must be atleast 3.");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// Create a staging buffer of the required size on the GPU
		m_Device.CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		// Map the memory from the CPU to the buffer created on the GPU
		vkMapMemory(m_Device.Device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		// Copy the data to the CPU which will get sent to the GPU
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		// Unmap the link between the CPU and GPU since the data on the CPU is no longer needed
		vkUnmapMemory(m_Device.Device(), stagingBufferMemory);

		// Create a vertex buffer of the required size on the GPU
		m_Device.CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer,
			m_VertexBufferMemory);

		// Copy the data from the staging buffer into the vertex buffer
		m_Device.CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.Device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.Device(), stagingBufferMemory, nullptr);
	}

	void VEModel::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_IndexCount = static_cast<uint32_t>(indices.size());

		// If a non empty vector of indices is provided, then use an index buffer for rendering the model
		m_HasIndexBuffer = m_IndexCount > 0;

		// If an index buffer isnt needed then return
		if (!m_HasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// Create a staging buffer of the required size on the GPU
		m_Device.CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		// Map the memory from the CPU to the buffer created on the GPU
		vkMapMemory(m_Device.Device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		// Copy the data to the CPU which will get sent to the GPU
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		// Unmap the link between the CPU and GPU since the data on the CPU is no longer needed
		vkUnmapMemory(m_Device.Device(), stagingBufferMemory);

		// Create a index buffer of the required size on the GPU
		m_Device.CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndexBuffer,
			m_IndexBufferMemory);

		// Copy the data from the staging buffer into the index buffer
		m_Device.CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.Device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.Device(), stagingBufferMemory, nullptr);
	}

	void VEModel::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
		}

	}

	void VEModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[]					= { m_VertexBuffer };
		VkDeviceSize offsets[]				= { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (m_HasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription> VEModel::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

		bindingDescriptions[0].binding		= 0;
		bindingDescriptions[0].stride		= sizeof(Vertex);
		bindingDescriptions[0].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VEModel::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding	= 0;
		attributeDescriptions[0].location	= 0;
		attributeDescriptions[0].format		= VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset		= offsetof(Vertex, Position);

		attributeDescriptions[1].binding	= 0;
		attributeDescriptions[1].location	= 1;
		attributeDescriptions[1].format		= VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset		= offsetof(Vertex, Color);

		return attributeDescriptions;
	}

	void VEModel::Builder::LoadModel(const std::string& filepath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filepath.c_str()))
		{
			throw std::runtime_error(warn + error);
		}

		Vertices.clear();
		Indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex = {};

				if (index.vertex_index >= 0)
				{
					vertex.Position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					auto colorIndex = 3 * index.vertex_index + 2;

					if (colorIndex < attrib.colors.size())
					{
						vertex.Color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex - 0]
						};
					}
					else
					{
						vertex.Color = { 1.0f, 1.0f, 1.0f };
					}
				}

				if (index.normal_index >= 0)
				{
					vertex.Normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.UV = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
					Vertices.push_back(vertex);
				}

				Indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}