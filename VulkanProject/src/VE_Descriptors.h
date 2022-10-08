#pragma once
#include "VE_Device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace VulkanEngine {

    class VEDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(VEDevice& device) : m_Device{ device } {}

            Builder& AddBinding(uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

            std::unique_ptr<VEDescriptorSetLayout> Build() const;

        private:
            VEDevice& m_Device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings{};
        };

        VEDescriptorSetLayout(VEDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VEDescriptorSetLayout();

        VEDescriptorSetLayout(const VEDescriptorSetLayout&) = delete;
        VEDescriptorSetLayout& operator=(const VEDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

    private:
        VEDevice& m_Device;
        VkDescriptorSetLayout m_DescriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

        friend class VEDescriptorWriter;
    };

    class VEDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(VEDevice& device) : m_Device{ device } {}

            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            std::unique_ptr<VEDescriptorPool> Build() const;

        private:
            VEDevice& m_Device;
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};
            uint32_t m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };

        VEDescriptorPool(VEDevice& device, uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VEDescriptorPool();

        VEDescriptorPool(const VEDescriptorPool&) = delete;
        VEDescriptorPool& operator=(const VEDescriptorPool&) = delete;

        bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool();

    private:
        VEDevice& m_Device;
        VkDescriptorPool m_DescriptorPool;

        friend class VEDescriptorWriter;
    };

    class VEDescriptorWriter
    {
    public:
        VEDescriptorWriter(VEDescriptorSetLayout& setLayout, VEDescriptorPool& pool);

        VEDescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VEDescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        VEDescriptorSetLayout& m_SetLayout;
        VEDescriptorPool& m_Pool;
        std::vector<VkWriteDescriptorSet> m_Writes;
    };

}