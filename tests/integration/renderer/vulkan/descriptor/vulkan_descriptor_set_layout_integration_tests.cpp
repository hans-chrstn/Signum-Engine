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

TEST_F(VulkanIntegrationTest, EmptyLayoutHasNoBindingDescriptions) {
    const VkDescriptorSetLayoutCreateInfo create_info =
        makeDescriptorSetLayoutCreateInfo({});

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    EXPECT_NE(layout.nativeHandle(), VK_NULL_HANDLE);
    EXPECT_TRUE(layout.bindingDescriptions().empty());
}

TEST_F(VulkanIntegrationTest, RetainsMultipleBindingDescriptions) {
    const std::array<VkDescriptorSetLayoutBinding, 2> bindings{
        VkDescriptorSetLayoutBinding{
            .binding = 3U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 4U,
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        },
        VkDescriptorSetLayoutBinding{
            .binding = 7U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 2U,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    const auto create_info = makeDescriptorSetLayoutCreateInfo(bindings);

    const Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    const auto descriptions = layout.bindingDescriptions();

    ASSERT_EQ(descriptions.size(), 2U);

    EXPECT_EQ(descriptions[0].binding, 3U);
    EXPECT_EQ(descriptions[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    EXPECT_EQ(descriptions[0].descriptor_count, 4U);
    EXPECT_EQ(descriptions[0].stage_flags,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    EXPECT_EQ(descriptions[1].binding, 7U);
    EXPECT_EQ(descriptions[1].descriptor_type,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(descriptions[1].descriptor_count, 2U);
    EXPECT_EQ(descriptions[1].stage_flags, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST_F(VulkanIntegrationTest, BindingMetadataOutlivesCreationDescriptions) {
    const Vulkan::VulkanDescriptorSetLayout layout =
        [this] -> Vulkan::VulkanDescriptorSetLayout {
        const std::array bindings{makeTextureBinding()};

        const auto create_info = makeDescriptorSetLayoutCreateInfo(bindings);

        return Vulkan::VulkanDescriptorSetLayout{m_Device, create_info};
    }();

    const auto descriptions = layout.bindingDescriptions();

    ASSERT_EQ(descriptions.size(), 1U);

    EXPECT_EQ(descriptions[0].binding, 0U);
    EXPECT_EQ(descriptions[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(descriptions[0].descriptor_count, 1U);
    EXPECT_EQ(descriptions[0].stage_flags, VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(VulkanIntegrationTest, MoveConstructionTransfersBindingMetadata) {
    const std::array bindings{makeTextureBinding()};

    const auto create_info = makeDescriptorSetLayoutCreateInfo(bindings);

    Vulkan::VulkanDescriptorSetLayout source{m_Device, create_info};

    const VkDescriptorSetLayout original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    Vulkan::VulkanDescriptorSetLayout destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);

    const auto descriptions = destination.bindingDescriptions();

    ASSERT_EQ(descriptions.size(), 1U);

    EXPECT_EQ(descriptions[0].binding, 0U);
    EXPECT_EQ(descriptions[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(descriptions[0].descriptor_count, 1U);
    EXPECT_EQ(descriptions[0].stage_flags, VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(VulkanIntegrationTest, MoveAssignmentReplacesBindingMetadata) {
    const std::array source_bindings{makeTextureBinding()};

    const std::array destination_bindings{
        VkDescriptorSetLayoutBinding{
            .binding = 5U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 2U,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    const auto source_info = makeDescriptorSetLayoutCreateInfo(source_bindings);
    const auto destination_info =
        makeDescriptorSetLayoutCreateInfo(destination_bindings);

    Vulkan::VulkanDescriptorSetLayout source{m_Device, source_info};
    Vulkan::VulkanDescriptorSetLayout destination{m_Device, destination_info};

    const VkDescriptorSetLayout source_handle = source.nativeHandle();
    const VkDescriptorSetLayout destination_handle = destination.nativeHandle();

    ASSERT_NE(destination_handle, VK_NULL_HANDLE);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), destination_handle);

    const auto descriptions = destination.bindingDescriptions();

    ASSERT_EQ(descriptions.size(), 1U);

    EXPECT_EQ(descriptions[0].binding, 0U);
    EXPECT_EQ(descriptions[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(descriptions[0].descriptor_count, 1U);
    EXPECT_EQ(descriptions[0].stage_flags, VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(VulkanIntegrationTest, SelfMovePreservesBindingMetadata) {
    const std::array bindings{makeTextureBinding()};

    const auto create_info = makeDescriptorSetLayoutCreateInfo(bindings);

    Vulkan::VulkanDescriptorSetLayout layout{m_Device, create_info};

    const VkDescriptorSetLayout original_handle = layout.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    auto &source = layout;
    layout = std::move(source);

    EXPECT_EQ(layout.nativeHandle(), original_handle);

    const auto descriptions = layout.bindingDescriptions();

    ASSERT_EQ(descriptions.size(), 1U);

    EXPECT_EQ(descriptions[0].binding, 0U);
    EXPECT_EQ(descriptions[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(descriptions[0].descriptor_count, 1U);
    EXPECT_EQ(descriptions[0].stage_flags, VK_SHADER_STAGE_FRAGMENT_BIT);
}
