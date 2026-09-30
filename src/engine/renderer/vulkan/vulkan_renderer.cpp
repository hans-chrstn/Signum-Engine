#include "vulkan_renderer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;
namespace Assertion = SNE::Engine::Core::Assertion;

namespace {
    [[nodiscard]] auto selectRequiredPhysicalDevice(VkInstance instance,
                                                    VkSurfaceKHR surface)
        -> Vulkan::PhysicalDeviceCandidate {
        const std::vector<VkPhysicalDevice> available_devices =
            Vulkan::enumeratePhysicalDevices(instance);

        const std::vector<Vulkan::PhysicalDeviceCandidate> candidates =
            Vulkan::createPhysicalDeviceCandidates(available_devices, surface);

        std::optional<Vulkan::PhysicalDeviceCandidate> candidate =
            Vulkan::selectPhysicalDevice(candidates);

        if (!candidate.has_value()) {
            throw Error::EngineError(
                Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
                "No suitable Vulkan physical device is available",
                "Obtain Vulkan Suitable Physical Device");
        }

        return std::move(candidate.value());
    }

    [[nodiscard]] auto requiredGraphicsQueueFamilyIndex(
        const Vulkan::PhysicalDeviceCandidate &candidate) -> std::uint32_t {
        const std::optional<std::uint32_t> &graphics_family =
            candidate.queue_family_indices.graphics_family;

        if (!graphics_family.has_value()) {
            Assertion::failAssertion(
                Assertion::AssertionType::Invariant, Error::Subsystem::Vulkan,
                "Selected physical device requires a graphics queue family");
        }

        return graphics_family.value();
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    VulkanRenderer::VulkanRenderer(
        const std::string &application_name, const Platform::Window &window,
        PresentationPreference presentation_preference,
        bool development_diagnostics_enabled)
        : m_Instance(application_name, development_diagnostics_enabled),
          m_Surface(m_Instance.nativeHandle(), window.nativeHandle()),
          m_PhysicalDeviceCandidate(selectRequiredPhysicalDevice(
              m_Instance.nativeHandle(), m_Surface.nativeHandle())),
          m_LogicalDeviceFeatureConfiguration(
              deriveLogicalDeviceFeatureConfiguration(
                  deriveLogicalDeviceFeatureRequest(presentation_preference),
                  m_PhysicalDeviceCandidate.capabilities)),
          m_Device(m_PhysicalDeviceCandidate.handle,
                   m_PhysicalDeviceCandidate.queue_family_indices,
                   deriveUniqueQueueFamilyRequests(
                       m_PhysicalDeviceCandidate.queue_family_indices),
                   m_LogicalDeviceFeatureConfiguration),
          m_Swapchain(
              m_PhysicalDeviceCandidate.handle, m_Device.nativeHandle(),
              m_Surface.nativeHandle(),
              m_PhysicalDeviceCandidate.queue_family_indices,
              window.framebufferSize(), presentation_preference,
              m_LogicalDeviceFeatureConfiguration.fifo_latest_ready_feature
                      .presentModeFifoLatestReady == VK_TRUE),
          m_CommandPool(
              m_Device.nativeHandle(),
              requiredGraphicsQueueFamilyIndex(m_PhysicalDeviceCandidate)) {}
} // namespace SNE::Engine::Renderer::Vulkan
