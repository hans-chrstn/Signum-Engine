#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_pool.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_updates.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image_view.hpp"
#include "engine/renderer/vulkan/memory/vulkan_sampler.hpp"
#include <array>
#include <gtest/gtest.h>
#include <span>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, UpdatesCombinedImageSamplerDescriptor) {
    const Vulkan::VulkanImageCreateInfo image_create_info{
        .extent = {.width = 4U, .height = 4U, .depth = 1U},
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
    };

    const Vulkan::VulkanImage image{memoryAllocator(), image_create_info};

    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image.nativeHandle();
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.levelCount = 1U;
    view_create_info.subresourceRange.layerCount = 1U;

    const Vulkan::VulkanImageView image_view{m_Device, view_create_info};

    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_NEAREST;
    sampler_create_info.minFilter = VK_FILTER_NEAREST;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.maxAnisotropy = 1.0F;

    const Vulkan::VulkanSampler sampler{m_Device, sampler_create_info};

    const VkDescriptorSetLayoutBinding texture_binding{
        .binding = 0U,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.bindingCount = 1U;
    layout_create_info.pBindings = &texture_binding;

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device,
                                                   layout_create_info};

    const VkDescriptorPoolSize pool_size{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1U,
    };

    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.maxSets = 1U;
    pool_create_info.poolSizeCount = 1U;
    pool_create_info.pPoolSizes = &pool_size;

    Vulkan::VulkanDescriptorPool pool{m_Device, pool_create_info};

    const std::array layouts{layout.nativeHandle()};
    const auto descriptor_sets = pool.allocateDescriptorSets(layouts);

    ASSERT_EQ(descriptor_sets.size(), 1U);
    ASSERT_NE(descriptor_sets[0], VK_NULL_HANDLE);

    const VkDescriptorImageInfo image_info{
        .sampler = sampler.nativeHandle(),
        .imageView = image_view.nativeHandle(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptor_sets[0];
    write.dstBinding = 0U;
    write.descriptorCount = 1U;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;

    const std::array writes{write};

    Vulkan::updateDescriptorSets(m_Device, writes);
}

TEST_F(VulkanIntegrationTest, AcceptsEmptyDescriptorWrites) {
    const std::span<const VkWriteDescriptorSet> writes{};

    Vulkan::updateDescriptorSets(m_Device, writes);
}

TEST(VulkanDescriptorUpdatesPreconditionTests, RejectsNullDevice) {
    const std::span<const VkWriteDescriptorSet> writes{};

    ASSERT_DEATH(Vulkan::updateDescriptorSets(VK_NULL_HANDLE, writes),
                 "Descriptor updates require a valid Vulkan device");
}

TEST_F(VulkanIntegrationTest, RejectsNullDestinationDescriptorSet) {
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = VK_NULL_HANDLE;
    write.descriptorCount = 1U;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const std::array writes{write};

    ASSERT_DEATH(
        Vulkan::updateDescriptorSets(m_Device, writes),
        "Descriptor updates require valid destination descriptor-set handles");
}

TEST_F(VulkanIntegrationTest, UpdatesUniformAndStorageBufferDescriptors) {
    constexpr VkDeviceSize kUniformSize = 64U;
    constexpr VkDeviceSize kStorageSize = 128U;

    const Vulkan::VulkanBufferCreateInfo uniform_create_info{
        .size = kUniformSize,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .memory_usage = SNE::Engine::Renderer::GpuMemoryUsage::Upload,
    };

    const Vulkan::VulkanBufferCreateInfo storage_create_info{
        .size = kStorageSize,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .memory_usage = SNE::Engine::Renderer::GpuMemoryUsage::Device,
    };

    const Vulkan::VulkanBuffer uniform_buffer{memoryAllocator(),
                                              uniform_create_info};
    const Vulkan::VulkanBuffer storage_buffer{memoryAllocator(),
                                              storage_create_info};

    VkDescriptorSetLayoutBinding uniform_binding{};
    uniform_binding.binding = 0U;
    uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_binding.descriptorCount = 1U;
    uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding storage_binding{};
    storage_binding.binding = 1U;
    storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_binding.descriptorCount = 1U;
    storage_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    const std::array<VkDescriptorSetLayoutBinding, 2> bindings{
        uniform_binding,
        storage_binding,
    };

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 2U;
    layout_info.pBindings = bindings.data();

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, layout_info};

    const std::array<VkDescriptorPoolSize, 2> pool_sizes{
        {
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1U},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1U},
        },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1U;
    pool_info.poolSizeCount = 2U;
    pool_info.pPoolSizes = pool_sizes.data();

    Vulkan::VulkanDescriptorPool pool{m_Device, pool_info};

    const std::array layouts{layout.nativeHandle()};
    const auto sets = pool.allocateDescriptorSets(layouts);

    ASSERT_EQ(sets.size(), 1U);
    ASSERT_NE(sets[0], VK_NULL_HANDLE);

    const VkDescriptorBufferInfo uniform_info{
        .buffer = uniform_buffer.nativeHandle(),
        .offset = 0U,
        .range = kUniformSize,
    };

    const VkDescriptorBufferInfo storage_info{
        .buffer = storage_buffer.nativeHandle(),
        .offset = 0U,
        .range = kStorageSize,
    };

    VkWriteDescriptorSet uniform_write{};
    uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniform_write.dstSet = sets[0];
    uniform_write.dstBinding = 0U;
    uniform_write.descriptorCount = 1U;
    uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_write.pBufferInfo = &uniform_info;

    VkWriteDescriptorSet storage_write{};
    storage_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_write.dstSet = sets[0];
    storage_write.dstBinding = 1U;
    storage_write.descriptorCount = 1U;
    storage_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_write.pBufferInfo = &storage_info;

    const std::array writes{uniform_write, storage_write};

    EXPECT_NO_THROW(Vulkan::updateDescriptorSets(m_Device, writes));
}
