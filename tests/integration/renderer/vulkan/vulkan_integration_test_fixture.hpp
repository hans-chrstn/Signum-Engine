#pragma once

#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/memory/vulkan_memory_allocator.hpp"
#include <gtest/gtest.h>
#include <optional>
#include <vulkan/vulkan.h>

class VulkanIntegrationTest : public ::testing::Test {
  protected:
    VkInstance m_Instance{VK_NULL_HANDLE};
    VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
    SNE::Engine::Renderer::Vulkan::SelectedQueueFamily m_SelectedQueueFamily{};
    VkDevice m_Device{VK_NULL_HANDLE};
    std::optional<SNE::Engine::Renderer::Vulkan::VulkanMemoryAllocator>
        m_MemoryAllocator;
    VkQueue m_GraphicsQueue{VK_NULL_HANDLE};
    std::optional<SNE::Engine::Renderer::Vulkan::VulkanImmediateSubmission>
        m_ImmediateSubmission;

    auto SetUp() -> void override;
    auto TearDown() -> void override;

    [[nodiscard]] auto memoryAllocator()
        -> SNE::Engine::Renderer::Vulkan::VulkanMemoryAllocator &;
    [[nodiscard]] auto immediateSubmission()
        -> SNE::Engine::Renderer::Vulkan::VulkanImmediateSubmission &;
};
