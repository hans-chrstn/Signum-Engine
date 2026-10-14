#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_pool.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    [[nodiscard]] auto makeDescriptorPoolCreateInfo(
        std::span<const VkDescriptorPoolSize> pool_sizes,
        std::uint32_t max_sets) -> VkDescriptorPoolCreateInfo {
        VkDescriptorPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.maxSets = max_sets;
        create_info.poolSizeCount =
            static_cast<std::uint32_t>(pool_sizes.size());
        create_info.pPoolSizes = pool_sizes.data();

        return create_info;
    }

    [[nodiscard]] auto makeTexturePoolSize() -> VkDescriptorPoolSize {
        return {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U,
        };
    }

    static_assert(!std::is_copy_constructible_v<Vulkan::VulkanDescriptorPool>);
    static_assert(!std::is_copy_assignable_v<Vulkan::VulkanDescriptorPool>);
    static_assert(
        std::is_nothrow_move_constructible_v<Vulkan::VulkanDescriptorPool>);
    static_assert(
        std::is_nothrow_move_assignable_v<Vulkan::VulkanDescriptorPool>);
} // namespace

TEST_F(VulkanIntegrationTest, CreatesTextureDescriptorPool) {
    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    const Vulkan::VulkanDescriptorPool pool{m_Device, create_info};

    EXPECT_NE(pool.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesDescriptorPoolWithMultipleTypes) {
    const std::array<VkDescriptorPoolSize, 2> pool_sizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 4U,
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 2U,
        },
    };

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 2U);

    const Vulkan::VulkanDescriptorPool pool{m_Device, create_info};

    EXPECT_NE(pool.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, AllocatesDescriptorSetFromPool) {
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

    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo pool_create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    const Vulkan::VulkanDescriptorPool pool{m_Device, pool_create_info};

    const VkDescriptorSetLayout layout_handle = layout.nativeHandle();

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = pool.nativeHandle();
    allocate_info.descriptorSetCount = 1U;
    allocate_info.pSetLayouts = &layout_handle;

    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};

    const VkResult result =
        vkAllocateDescriptorSets(m_Device, &allocate_info, &descriptor_set);

    ASSERT_EQ(result, VK_SUCCESS);
    EXPECT_NE(descriptor_set, VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, MoveConstructsDescriptorPool) {
    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    Vulkan::VulkanDescriptorPool source{m_Device, create_info};

    const VkDescriptorPool original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    const Vulkan::VulkanDescriptorPool destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, MoveAssignsDescriptorPool) {
    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    Vulkan::VulkanDescriptorPool source{m_Device, create_info};
    Vulkan::VulkanDescriptorPool destination{m_Device, create_info};

    const VkDescriptorPool source_handle = source.nativeHandle();
    const VkDescriptorPool destination_handle = destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);
}

TEST(VulkanDescriptorPoolPreconditionTests, RejectsNullDevice) {
    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    ASSERT_DEATH(static_cast<void>(
                     Vulkan::VulkanDescriptorPool{VK_NULL_HANDLE, create_info}),
                 "VulkanDescriptorPool requires a valid Vulkan device");
}

TEST_F(VulkanIntegrationTest, RejectsInvalidDescriptorPoolStructureType) {
    const std::array pool_sizes{makeTexturePoolSize()};

    VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanDescriptorPool{m_Device, create_info}),
        "requires a valid descriptor-pool creation structure type");
}

TEST_F(VulkanIntegrationTest, RejectsZeroMaximumSets) {
    const std::array pool_sizes{makeTexturePoolSize()};

    const VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 0U);

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanDescriptorPool{m_Device, create_info}),
        "requires at least one allocatable descriptor set");
}

TEST_F(VulkanIntegrationTest, RejectsMissingPoolSizeDescriptions) {
    const std::array pool_sizes{makeTexturePoolSize()};

    VkDescriptorPoolCreateInfo create_info =
        makeDescriptorPoolCreateInfo(pool_sizes, 1U);

    create_info.pPoolSizes = nullptr;

    ASSERT_DEATH(
        static_cast<void>(Vulkan::VulkanDescriptorPool{m_Device, create_info}),
        "requires pool-size descriptions when pool size count is non-zero");
}

TEST_F(VulkanIntegrationTest, AllocatesTextureSetThroughDescriptorPool) {
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

    const std::array pool_sizes{makeTexturePoolSize()};

    Vulkan::VulkanDescriptorPool pool{
        m_Device, makeDescriptorPoolCreateInfo(pool_sizes, 1U)};

    const std::array layouts{layout.nativeHandle()};

    const auto descriptor_sets = pool.allocateDescriptorSets(layouts);

    ASSERT_EQ(descriptor_sets.size(), layouts.size());
    EXPECT_NE(descriptor_sets[0], VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, AllocatesMultipleDescriptorSetsThroughPool) {
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

    const std::array pool_sizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 2U,
        },
    };

    Vulkan::VulkanDescriptorPool pool{
        m_Device, makeDescriptorPoolCreateInfo(pool_sizes, 2U)};

    const std::array<VkDescriptorSetLayout, 2> layouts{
        layout.nativeHandle(),
        layout.nativeHandle(),
    };

    const auto descriptor_sets = pool.allocateDescriptorSets(layouts);

    ASSERT_EQ(descriptor_sets.size(), layouts.size());
    EXPECT_NE(descriptor_sets[0], VK_NULL_HANDLE);
    EXPECT_NE(descriptor_sets[1], VK_NULL_HANDLE);
    EXPECT_NE(descriptor_sets[0], descriptor_sets[1]);
}

TEST_F(VulkanIntegrationTest, RejectsEmptyDescriptorSetAllocation) {
    const std::array pool_sizes{makeTexturePoolSize()};

    Vulkan::VulkanDescriptorPool pool{
        m_Device, makeDescriptorPoolCreateInfo(pool_sizes, 1U)};

    const std::span<const VkDescriptorSetLayout> layouts{};

    ASSERT_DEATH(static_cast<void>(pool.allocateDescriptorSets(layouts)),
                 "requires at least one descriptor-set layout");
}

TEST_F(VulkanIntegrationTest, RejectsNullDescriptorSetLayoutHandle) {
    const std::array pool_sizes{makeTexturePoolSize()};

    Vulkan::VulkanDescriptorPool pool{
        m_Device, makeDescriptorPoolCreateInfo(pool_sizes, 1U)};

    const std::array<VkDescriptorSetLayout, 1> layouts{
        VK_NULL_HANDLE,
    };

    ASSERT_DEATH(static_cast<void>(pool.allocateDescriptorSets(layouts)),
                 "requires valid descriptor-set layout handles");
}
