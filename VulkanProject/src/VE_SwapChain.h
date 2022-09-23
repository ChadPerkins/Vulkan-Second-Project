#pragma once
#pragma once

#include "VE_Device.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace VulkanEngine {

    class VESwapChain {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        VESwapChain(VEDevice& deviceRef, VkExtent2D windowExtent);
        ~VESwapChain();

        VESwapChain(const VESwapChain&) = delete;
        void operator=(const VESwapChain&) = delete;

        VkFramebuffer GetFrameBuffer(int index) { return m_SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() { return m_RenderPass; }
        VkImageView GetImageView(int index) { return m_SwapChainImageViews[index]; }
        size_t ImageCount() { return m_SwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }
        uint32_t Width() { return m_SwapChainExtent.width; }
        uint32_t Height() { return m_SwapChainExtent.height; }

        float ExtentAspectRatio() {
            return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
        }
        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

    private:
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    private:
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        VEDevice& m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };

}