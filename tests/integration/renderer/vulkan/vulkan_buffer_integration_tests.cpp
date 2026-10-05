#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_immediate_submission.hpp"
#include "engine/renderer/vulkan/vulkan_memory_allocator.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_readback.hpp"
#include "engine/renderer/vulkan/vulkan_upload.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

class VulkanBufferIntegrationTests : public ::testing::Test {
  protected:
    VkInstance m_Instance{VK_NULL_HANDLE};
    VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
    Vulkan::SelectedQueueFamily m_SelectedQueueFamily{};
    VkDevice m_Device{VK_NULL_HANDLE};
    std::optional<Vulkan::VulkanMemoryAllocator> m_MemoryAllocator;

    auto SetUp() -> void override;
    auto TearDown() -> void override;
};

auto VulkanBufferIntegrationTests::SetUp() -> void {
    std::uint32_t supported_api_version{};
    const VkResult api_result =
        vkEnumerateInstanceVersion(&supported_api_version);

    if (api_result != VK_SUCCESS) {
        FAIL() << "Failed to query Vulkan loader API version";
    }

    if (!Vulkan::supportsRequiredApiVersion(supported_api_version)) {
        GTEST_SKIP() << "Vulkan loader does not support Signum's required API "
                        "version";
    }

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = Vulkan::kRequiredApiVersion;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;

    const VkResult result =
        vkCreateInstance(&create_info, nullptr, &m_Instance);

    ASSERT_EQ(result, VK_SUCCESS);

    const std::vector<VkPhysicalDevice> devices =
        Vulkan::enumeratePhysicalDevices(m_Instance);

    if (devices.empty()) {
        GTEST_SKIP() << "No Vulkan physical devices available";
    }

    m_PhysicalDevice = devices.front();

    const std::vector<VkQueueFamilyProperties> queue_family_properties =
        Vulkan::queryQueueFamilyProperties(m_PhysicalDevice);

    const std::optional<std::uint32_t> graphics_family =
        Vulkan::findGraphicsQueueFamily(queue_family_properties);

    if (!graphics_family.has_value()) {
        FAIL() << "Failed to find a graphics queue family";
    }

    m_SelectedQueueFamily.family_index = graphics_family.value();
    m_SelectedQueueFamily.available_queue_count =
        queue_family_properties[graphics_family.value()].queueCount;

    const float priority = 1.0F;
    VkDeviceQueueCreateInfo device_queue_info{};
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.queueFamilyIndex = m_SelectedQueueFamily.family_index;
    device_queue_info.queueCount = 1U;
    device_queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1U;
    device_info.pQueueCreateInfos = &device_queue_info;

    const VkResult device_result =
        vkCreateDevice(m_PhysicalDevice, &device_info, nullptr, &m_Device);

    ASSERT_EQ(device_result, VK_SUCCESS);

    m_MemoryAllocator.emplace(m_Instance, m_PhysicalDevice, m_Device,
                              Vulkan::kRequiredApiVersion);
}

auto VulkanBufferIntegrationTests::TearDown() -> void {
    m_MemoryAllocator.reset();
    if (m_Device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_Device, nullptr);
    }

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
    }

    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_Instance = VK_NULL_HANDLE;
}

TEST_F(VulkanBufferIntegrationTests, CreatesAndDestroysVmaBackedBuffer) {
    ASSERT_NE(m_Instance, VK_NULL_HANDLE);
    ASSERT_NE(m_PhysicalDevice, VK_NULL_HANDLE);
    ASSERT_NE(m_Device, VK_NULL_HANDLE);

    const Vulkan::VulkanBufferCreateInfo buffer_create_info{
        .size = 256U,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    if (!m_MemoryAllocator.has_value()) {
        FAIL() << "Failed to get a value for memory allocator";
    }

    Vulkan::VulkanBuffer buffer = Vulkan::VulkanBuffer(
        m_MemoryAllocator.value().nativeHandle(), buffer_create_info);
}

TEST_F(VulkanBufferIntegrationTests, WritesToVmaBackedUploadBuffer) {
    ASSERT_NE(m_Instance, VK_NULL_HANDLE);
    ASSERT_NE(m_PhysicalDevice, VK_NULL_HANDLE);
    ASSERT_NE(m_Device, VK_NULL_HANDLE);

    if (!m_MemoryAllocator.has_value()) {
        FAIL() << "Failed to get a value for memory allocator";
    }

    std::array<std::byte, 4> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    Vulkan::VulkanBufferCreateInfo buffer_create_info{};
    buffer_create_info.size = static_cast<VkDeviceSize>(bytes.size());
    buffer_create_info.memory_usage = Renderer::GpuMemoryUsage::Upload;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Vulkan::VulkanBuffer buffer = Vulkan::VulkanBuffer(
        m_MemoryAllocator.value().nativeHandle(), buffer_create_info);

    buffer.write(bytes);
}

TEST_F(VulkanBufferIntegrationTests, UploadsAndReadsBackBufferData) {
    ASSERT_NE(m_Instance, VK_NULL_HANDLE);
    ASSERT_NE(m_PhysicalDevice, VK_NULL_HANDLE);
    ASSERT_NE(m_Device, VK_NULL_HANDLE);

    if (!m_MemoryAllocator.has_value()) {
        FAIL() << "Failed to get a value for memory allocator";
    }

    std::array<std::byte, 4> input_bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    std::array<std::byte, 4> output_bytes{
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
    };

    VkQueue graphics_queue{VK_NULL_HANDLE};

    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &graphics_queue);

    ASSERT_NE(graphics_queue, VK_NULL_HANDLE);

    Vulkan::VulkanImmediateSubmission immediate_submission =
        Vulkan::VulkanImmediateSubmission(m_Device, graphics_queue,
                                          m_SelectedQueueFamily.family_index);

    const auto input_size = static_cast<VkDeviceSize>(input_bytes.size());

    Vulkan::VulkanBufferCreateInfo buffer_create_info{};
    buffer_create_info.size = input_size;
    buffer_create_info.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.memory_usage = Renderer::GpuMemoryUsage::Device;

    Vulkan::VulkanBuffer device_buffer = Vulkan::VulkanBuffer(
        m_MemoryAllocator.value().nativeHandle(), buffer_create_info);

    Vulkan::uploadBufferData(m_MemoryAllocator.value().nativeHandle(),
                             immediate_submission, device_buffer, input_bytes);

    Vulkan::readBufferData(m_MemoryAllocator.value().nativeHandle(),
                           immediate_submission, device_buffer, output_bytes);

    EXPECT_EQ(input_bytes, output_bytes);
}
