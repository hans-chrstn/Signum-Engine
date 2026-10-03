#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <cstdint>
#include <string>
#define VMA_IMPLEMENTATION
#include "vulkan_memory_allocator.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    VulkanMemoryAllocator::VulkanMemoryAllocator(
        VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
        std::uint32_t api_version) {
        VmaAllocatorCreateInfo allocator_create_info{};
        allocator_create_info.instance = instance;
        allocator_create_info.physicalDevice = physical_device;
        allocator_create_info.device = device;
        allocator_create_info.vulkanApiVersion = api_version;

        const VkResult result =
            vmaCreateAllocator(&allocator_create_info, &m_Allocator);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanMemoryAllocatorCreationFailed,
                "Failed to create Vulkan memory allocator",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Memory Allocator");
        }
    }

    VulkanMemoryAllocator::~VulkanMemoryAllocator() noexcept {
        if (m_Allocator != nullptr) {
            vmaDestroyAllocator(m_Allocator);
        }
        m_Allocator = nullptr;
    }

    auto VulkanMemoryAllocator::nativeHandle() const noexcept -> VmaAllocator {
        return m_Allocator;
    }
} // namespace SNE::Engine::Renderer::Vulkan
