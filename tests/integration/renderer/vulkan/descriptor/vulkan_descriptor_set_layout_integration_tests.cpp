#include "../vulkan_integration_test_fixture.hpp"
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
    [[nodiscard]] auto makeDescriptorSetLayoutCreateInfo(
        std::span<const VkDescriptorSetLayoutBinding> bindings)
        -> VkDescriptorSetLayoutCreateInfo {
        VkDescriptorSetLayoutCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
        create_info.pBindings = bindings.data();

        return create_info;
    }

    [[nodiscard]] auto makeTextureBinding() -> VkDescriptorSetLayoutBinding {
        return {
            .binding = 0U,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };
    }

    static_assert(
        !std::is_copy_constructible_v<Vulkan::VulkanDescriptorSetLayout>);
    static_assert(
        !std::is_copy_assignable_v<Vulkan::VulkanDescriptorSetLayout>);
    static_assert(std::is_nothrow_move_constructible_v<
                  Vulkan::VulkanDescriptorSetLayout>);
    static_assert(
        std::is_nothrow_move_assignable_v<Vulkan::VulkanDescriptorSetLayout>);
} // namespace

TEST_F(VulkanIntegrationTest, CreatesEmptyDescriptorSetLayout) {
    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo({});

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    EXPECT_NE(layout.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesTextureDescriptorSetLayout) {
    const std::array bindings{makeTextureBinding()};

    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo(bindings);

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    EXPECT_NE(layout.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, CreatesDescriptorSetLayoutWithMultipleBindings) {
    const std::array<VkDescriptorSetLayoutBinding, 2> bindings{
        VkDescriptorSetLayoutBinding{
            .binding = 0U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
        VkDescriptorSetLayoutBinding{
            .binding = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo(bindings);

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    EXPECT_NE(layout.nativeHandle(), VK_NULL_HANDLE);
}

TEST_F(VulkanIntegrationTest, MoveConstructsDescriptorSetLayout) {
    const std::array bindings{makeTextureBinding()};

    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo(bindings);

    Vulkan::VulkanDescriptorSetLayout source{m_Device, create_info};

    const VkDescriptorSetLayout original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    const Vulkan::VulkanDescriptorSetLayout destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, MoveAssignsDescriptorSetLayout) {
    const std::array bindings{makeTextureBinding()};

    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo(bindings);

    Vulkan::VulkanDescriptorSetLayout source{m_Device, create_info};
    Vulkan::VulkanDescriptorSetLayout destination{m_Device, create_info};

    const VkDescriptorSetLayout source_handle = source.nativeHandle();
    const VkDescriptorSetLayout destination_handle = destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);
}

TEST_F(VulkanIntegrationTest, SelfMoveAssignmentPreservesDescriptorSetLayout) {
    const std::array bindings{makeTextureBinding()};

    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo(bindings);

    Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    const VkDescriptorSetLayout original_handle = layout.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    auto &source = layout;
    layout = std::move(source);

    EXPECT_EQ(layout.nativeHandle(), original_handle);
}

TEST(VulkanDescriptorSetLayoutPreconditionTests, RejectsNullDevice) {
    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo({});

    ASSERT_DEATH(static_cast<void>(Vulkan::VulkanDescriptorSetLayout{
                     VK_NULL_HANDLE, create_info}),
                 "VulkanDescriptorSetLayout requires a valid Vulkan device");
}

TEST_F(VulkanIntegrationTest, RejectsInvalidDescriptorSetLayoutStructureType) {
    VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo({});

    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    ASSERT_DEATH(
        static_cast<void>(
            Vulkan::VulkanDescriptorSetLayout{m_Device, create_info}),
        "requires a valid descriptor-set layout creation structure type");
}

TEST_F(VulkanIntegrationTest, RejectsMissingBindingDescriptions) {
    VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo({});

    create_info.bindingCount = 1U;

    ASSERT_DEATH(
        static_cast<void>(
            Vulkan::VulkanDescriptorSetLayout{m_Device, create_info}),
        "requires binding descriptions when binding count is non-zero");
}
