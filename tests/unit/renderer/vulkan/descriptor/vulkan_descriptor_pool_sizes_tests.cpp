#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_pool.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    [[nodiscard]] auto makeBinding(std::uint32_t binding, VkDescriptorType type,
                                   std::uint32_t count)
        -> VkDescriptorSetLayoutBinding {
        return {
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = count,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };
    }
} // namespace

TEST(VulkanDescriptorPoolSizesTests, AcceptsEmptyRequirements) {
    const std::array<Vulkan::VulkanDescriptorSetLayoutRequirements, 0>
        requirements{};

    const auto sizes = Vulkan::calculateVkDescriptorPoolSizes(requirements);

    EXPECT_TRUE(sizes.empty());
}

TEST(VulkanDescriptorPoolSizesTests, AcceptsSetsWithoutBindings) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings = {},
        },
    };

    const auto sizes = Vulkan::calculateVkDescriptorPoolSizes(requirements);

    EXPECT_TRUE(sizes.empty());
}

TEST(VulkanDescriptorPoolSizesTests, CombinesDescriptorCountsAcrossSets) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2U),
                    makeBinding(1U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3U),
                },
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 2U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4U),
                },
        },
    };

    const auto sizes = Vulkan::calculateVkDescriptorPoolSizes(requirements);

    ASSERT_EQ(sizes.size(), 1U);
    EXPECT_EQ(sizes[0].type, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(sizes[0].descriptorCount, 9U);
}

TEST(VulkanDescriptorPoolSizesTests, KeepsDifferentDescriptorTypesSeparate) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                2U),
                    makeBinding(1U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3U),
                },
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 1U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                1U),
                },
        },
    };

    const auto sizes = Vulkan::calculateVkDescriptorPoolSizes(requirements);

    ASSERT_EQ(sizes.size(), 2U);

    EXPECT_EQ(sizes[0].type, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(sizes[0].descriptorCount, 3U);

    EXPECT_EQ(sizes[1].type, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(sizes[1].descriptorCount, 3U);
}

TEST(VulkanDescriptorPoolSizesTests, RejectsDescriptorCountOverflow) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                std::numeric_limits<std::uint32_t>::max()),
                },
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 1U,
            .bindings =
                {
                    makeBinding(0U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U),
                },
        },
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::calculateVkDescriptorPoolSizes(requirements)),
        "Vulkan descriptor pool descriptor count exceeds uint32_t");
}
