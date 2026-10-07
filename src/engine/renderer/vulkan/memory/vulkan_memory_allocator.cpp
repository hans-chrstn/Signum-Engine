#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#define VMA_IMPLEMENTATION
#include "vulkan_memory_allocator.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    VulkanMemoryAllocator::VulkanMemoryAllocator(
        VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
        std::uint32_t api_version, bool memory_budget_extension_enabled) {
        if (instance == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanMemoryAllocator requires a valid Vulkan instance");
        }

        if (physical_device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanMemoryAllocator requires a valid Vulkan physical "
                "device");
        }

        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanMemoryAllocator requires a valid Vulkan logical device");
        }

        VmaAllocatorCreateInfo allocator_create_info{};
        allocator_create_info.instance = instance;
        allocator_create_info.physicalDevice = physical_device;
        allocator_create_info.device = device;
        allocator_create_info.vulkanApiVersion = api_version;
        if (memory_budget_extension_enabled) {
            allocator_create_info.flags |=
                VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        }
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

        if (m_Allocator == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Postcondition,
                Core::Error::Subsystem::Vulkan,
                "Successful VMA allocator creation must produce a valid "
                "allocator");
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

    auto VulkanMemoryAllocator::queryMemoryHeapBudgets() const
        -> std::vector<VulkanMemoryHeapBudget> {
        std::array<VmaBudget, VK_MAX_MEMORY_HEAPS> budgets{{}};
        vmaGetHeapBudgets(m_Allocator, budgets.data());
        const VkPhysicalDeviceMemoryProperties *memory_properties{};
        vmaGetMemoryProperties(m_Allocator, &memory_properties);
        const std::uint32_t heap_count = memory_properties->memoryHeapCount;
        std::vector<VulkanMemoryHeapBudget> memory_heap_budgets{};
        memory_heap_budgets.reserve(heap_count);

        for (std::size_t i{}; i < heap_count; ++i) {
            VulkanMemoryHeapBudget budget{};
            budget.usage = budgets[i].usage;
            budget.budget = budgets[i].budget;
            memory_heap_budgets.push_back(budget);
        }
        return memory_heap_budgets;
    }

    auto VulkanMemoryAllocator::queryMemoryAllocationStatistics() const
        -> VulkanMemoryAllocationStatistics {
        VmaTotalStatistics total_statistics{};
        vmaCalculateStatistics(m_Allocator, &total_statistics);
        const VmaStatistics &statistics = total_statistics.total.statistics;

        VulkanMemoryAllocationStatistics memory_allocation_statistics{};
        memory_allocation_statistics.allocation_bytes =
            statistics.allocationBytes;
        memory_allocation_statistics.allocation_count =
            statistics.allocationCount;
        memory_allocation_statistics.block_count = statistics.blockCount;
        return memory_allocation_statistics;
    }
} // namespace SNE::Engine::Renderer::Vulkan
