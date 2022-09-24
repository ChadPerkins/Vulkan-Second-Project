#pragma once
#include "VE_Device.h"

#include <string>
#include <vector>

namespace VulkanEngine {

	struct PipelineConfigInfo
	{
		VkViewport Viewport;
		VkRect2D Scissor;
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo RasterizationInfo;
		VkPipelineMultisampleStateCreateInfo MultisampleInfo;
		VkPipelineColorBlendAttachmentState ColorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
		VkPipelineLayout PipelineLayout		= nullptr;
		VkRenderPass RenderPass				= nullptr;
		uint32_t Subpass					= 0;
	};

	class VEPipeline
	{
	public:
		VEPipeline(VEDevice& device,
			const std::string& vertShaderPath,
			const std::string& fragShaderPath,
			const PipelineConfigInfo configInfo);
		~VEPipeline();

		VEPipeline(const VEPipeline&) = delete;
		void operator=(const VEPipeline&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);

	private:
		static std::vector<char> ReadFile(const std::string& filepath);

		void CreateGraphicsPipeline(const std::string& vertShaderPath,
			const std::string& fragShaderPath,
			const PipelineConfigInfo configInfo);

		void CreateShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule);

	private:
		VEDevice& m_Device;
		VkPipeline m_GraphicsPipeline;
		VkShaderModule m_VertShaderModule;
		VkShaderModule m_FragShaderModule;
	};
}


