#include "VE_Descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace VulkanEngine {

    // *************** Descriptor Set Layout Builder *********************

    VEDescriptorSetLayout::Builder& VEDescriptorSetLayout::Builder::AddBinding(uint32_t binding,
        VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count)
    {
        assert(m_Bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_Bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<VEDescriptorSetLayout> VEDescriptorSetLayout::Builder::Build() const
    {
        return std::make_unique<VEDescriptorSetLayout>(m_Device, m_Bindings);
    }

    // *************** Descriptor Set Layout *********************

    VEDescriptorSetLayout::VEDescriptorSetLayout(VEDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : m_Device{ device }, m_Bindings{ bindings }
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};

        for (auto kv : bindings)
        {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(m_Device.Device(), &descriptorSetLayoutInfo,
            nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VEDescriptorSetLayout::~VEDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device.Device(), m_DescriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    VEDescriptorPool::Builder& VEDescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
    {
        m_PoolSizes.push_back({ descriptorType, count });
        return *this;
    }

    VEDescriptorPool::Builder& VEDescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
    {
        m_PoolFlags = flags;
        return *this;
    }
    VEDescriptorPool::Builder& VEDescriptorPool::Builder::SetMaxSets(uint32_t count)
    {
        m_MaxSets = count;
        return *this;
    }

    std::unique_ptr<VEDescriptorPool> VEDescriptorPool::Builder::Build() const
    {
        return std::make_unique<VEDescriptorPool>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
    }

    // *************** Descriptor Pool *********************

    VEDescriptorPool::VEDescriptorPool(VEDevice& device, uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_Device{ device }
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(m_Device.Device(), &descriptorPoolInfo, nullptr, &m_DescriptorPool)
            != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    VEDescriptorPool::~VEDescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device.Device(), m_DescriptorPool, nullptr);
    }

    bool VEDescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(m_Device.Device(), &allocInfo, &descriptor) != VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

    void VEDescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const 
    {
        vkFreeDescriptorSets(m_Device.Device(), m_DescriptorPool,
            static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void VEDescriptorPool::ResetPool() 
    {
        vkResetDescriptorPool(m_Device.Device(), m_DescriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    VEDescriptorWriter::VEDescriptorWriter(VEDescriptorSetLayout& setLayout, VEDescriptorPool& pool)
        : m_SetLayout{ setLayout }, m_Pool{ pool }
    {
    
    }

    VEDescriptorWriter& VEDescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    VEDescriptorWriter& VEDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    bool VEDescriptorWriter::Build(VkDescriptorSet& set)
    {
        bool success = m_Pool.AllocateDescriptorSet(m_SetLayout.GetDescriptorSetLayout(), set);

        if (!success)
        {
            return false;
        }

        Overwrite(set);
        return true;
    }

    void VEDescriptorWriter::Overwrite(VkDescriptorSet& set) {
        for (auto& write : m_Writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(m_Pool.m_Device.Device(), m_Writes.size(), m_Writes.data(), 0, nullptr);
    }

}
