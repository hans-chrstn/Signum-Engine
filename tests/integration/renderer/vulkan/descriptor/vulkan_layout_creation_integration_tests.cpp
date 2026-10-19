#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    [[nodiscard]] auto makeTextureBinding() -> VkDescriptorSetLayoutBinding {
        return {
            .binding = 0U,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };
    }

    [[nodiscard]] auto makeUniformBufferBinding()
        -> VkDescriptorSetLayoutBinding {
        return {
            .binding = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 2U,
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };
    }

    [[nodiscard]] auto supportedSetLimit(VkPhysicalDevice physical_device)
        -> std::uint32_t {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        return properties.limits.maxBoundDescriptorSets;
    }
} // namespace

TEST(VulkanDescriptorSetLayoutCreationPreconditionTests, RejectsNullDevice) {
    const std::array<Vulkan::VulkanDescriptorSetLayoutRequirements, 0>
        requirements{};

    ASSERT_DEATH(
        static_cast<void>(Vulkan::createVulkanDescriptorSetLayouts(
            VK_NULL_HANDLE, requirements, 4U)),
        "Vulkan descriptor-set layout creation requires a valid logical "
        "device");
}

TEST_F(VulkanIntegrationTest, CreatesNoLayoutsForEmptyRequirements) {
    const std::array<Vulkan::VulkanDescriptorSetLayoutRequirements, 0>
        requirements{};

    const auto layouts =
        Vulkan::createVulkanDescriptorSetLayouts(m_Device, requirements, 0U);

    EXPECT_TRUE(layouts.empty());
}

TEST_F(VulkanIntegrationTest, CreatesLayoutWithMultipleBindings) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings =
                {
                    makeTextureBinding(),
                    makeUniformBufferBinding(),
                },
        },
    };

    const auto layouts = Vulkan::createVulkanDescriptorSetLayouts(
        m_Device, requirements, supportedSetLimit(m_PhysicalDevice));

    ASSERT_EQ(layouts.size(), 1U);
    EXPECT_NE(layouts[0].nativeHandle(), VK_NULL_HANDLE);

    const auto bindings = layouts[0].bindingDescriptions();

    ASSERT_EQ(bindings.size(), 2U);

    EXPECT_EQ(bindings[0].binding, 0U);
    EXPECT_EQ(bindings[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    EXPECT_EQ(bindings[0].descriptor_count, 1U);
    EXPECT_EQ(bindings[0].stage_flags, VK_SHADER_STAGE_FRAGMENT_BIT);

    EXPECT_EQ(bindings[1].binding, 1U);
    EXPECT_EQ(bindings[1].descriptor_type, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    EXPECT_EQ(bindings[1].descriptor_count, 2U);
    EXPECT_EQ(bindings[1].stage_flags,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(VulkanIntegrationTest, CreatesEmptyLayoutsForMissingSetIndices) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings = {makeTextureBinding()},
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 2U,
            .bindings = {makeUniformBufferBinding()},
        },
    };

    const auto layouts = Vulkan::createVulkanDescriptorSetLayouts(
        m_Device, requirements, supportedSetLimit(m_PhysicalDevice));

    ASSERT_EQ(layouts.size(), 3U);

    for (const auto &layout : layouts) {
        EXPECT_NE(layout.nativeHandle(), VK_NULL_HANDLE);
    }

    ASSERT_EQ(layouts[0].bindingDescriptions().size(), 1U);
    EXPECT_TRUE(layouts[1].bindingDescriptions().empty());
    ASSERT_EQ(layouts[2].bindingDescriptions().size(), 1U);

    EXPECT_EQ(layouts[0].bindingDescriptions()[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    EXPECT_EQ(layouts[2].bindingDescriptions()[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

TEST_F(VulkanIntegrationTest, CreatesEmptyLayoutsBeforeFirstRequiredSet) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 2U,
            .bindings = {makeTextureBinding()},
        },
    };

    const auto layouts = Vulkan::createVulkanDescriptorSetLayouts(
        m_Device, requirements, supportedSetLimit(m_PhysicalDevice));

    ASSERT_EQ(layouts.size(), 3U);

    EXPECT_TRUE(layouts[0].bindingDescriptions().empty());
    EXPECT_TRUE(layouts[1].bindingDescriptions().empty());

    ASSERT_EQ(layouts[2].bindingDescriptions().size(), 1U);
    EXPECT_EQ(layouts[2].bindingDescriptions()[0].binding, 0U);
}

TEST_F(VulkanIntegrationTest, CreatesLayoutForSetAtSupportedLimit) {
    const std::uint32_t limit = supportedSetLimit(m_PhysicalDevice);

    ASSERT_GT(limit, 0U);

    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = limit - 1U,
            .bindings = {makeTextureBinding()},
        },
    };

    const auto layouts =
        Vulkan::createVulkanDescriptorSetLayouts(m_Device, requirements, limit);

    ASSERT_EQ(layouts.size(), limit);
    EXPECT_NE(layouts.back().nativeHandle(), VK_NULL_HANDLE);

    ASSERT_EQ(layouts.back().bindingDescriptions().size(), 1U);
    EXPECT_EQ(layouts.back().bindingDescriptions()[0].descriptor_type,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

TEST_F(VulkanIntegrationTest, RejectsDuplicateDescriptorSetIndices) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 1U,
            .bindings = {makeTextureBinding()},
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 1U,
            .bindings = {makeUniformBufferBinding()},
        },
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::createVulkanDescriptorSetLayouts(
            m_Device, requirements, supportedSetLimit(m_PhysicalDevice))),
        "must be ordered by strictly increasing set indices");
}

TEST_F(VulkanIntegrationTest, RejectsUnorderedDescriptorSetIndices) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 2U,
            .bindings = {makeTextureBinding()},
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings = {makeUniformBufferBinding()},
        },
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::createVulkanDescriptorSetLayouts(
            m_Device, requirements, supportedSetLimit(m_PhysicalDevice))),
        "must be ordered by strictly increasing set indices");
}

TEST_F(VulkanIntegrationTest, RejectsSetIndexAtDeviceLimit) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 2U,
            .bindings = {makeTextureBinding()},
        },
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::createVulkanDescriptorSetLayouts(
                     m_Device, requirements, 2U)),
                 "exceed the device's maximum bound descriptor-set count");
}

TEST_F(VulkanIntegrationTest, RejectsZeroMaximumForNonemptyRequirements) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings = {makeTextureBinding()},
        },
    };

    ASSERT_DEATH(static_cast<void>(Vulkan::createVulkanDescriptorSetLayouts(
                     m_Device, requirements, 0U)),
                 "exceed the device's maximum bound descriptor-set count");
}

TEST_F(VulkanIntegrationTest, ReturnedLayoutsRetainOwnershipAfterMove) {
    const std::array requirements{
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 0U,
            .bindings = {makeTextureBinding()},
        },
        Vulkan::VulkanDescriptorSetLayoutRequirements{
            .set = 1U,
            .bindings = {makeUniformBufferBinding()},
        },
    };

    auto layouts = Vulkan::createVulkanDescriptorSetLayouts(
        m_Device, requirements, supportedSetLimit(m_PhysicalDevice));

    ASSERT_EQ(layouts.size(), 2U);

    const VkDescriptorSetLayout first_handle = layouts[0].nativeHandle();
    const VkDescriptorSetLayout second_handle = layouts[1].nativeHandle();

    auto moved_layouts = std::move(layouts);

    ASSERT_EQ(moved_layouts.size(), 2U);

    EXPECT_EQ(moved_layouts[0].nativeHandle(), first_handle);
    EXPECT_EQ(moved_layouts[1].nativeHandle(), second_handle);

    EXPECT_EQ(moved_layouts[0].bindingDescriptions().size(), 1U);
    EXPECT_EQ(moved_layouts[1].bindingDescriptions().size(), 1U);
}
