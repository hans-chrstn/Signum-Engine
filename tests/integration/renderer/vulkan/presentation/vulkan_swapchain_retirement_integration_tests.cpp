#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/device/vulkan_device.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain_support.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <gtest/gtest.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vulkan/vulkan.h>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;

namespace {
    template <typename Operation>
    [[nodiscard]] auto captureSwapchainValidationOutput(Operation &&operation)
        -> std::pair<std::string, std::string> {
        testing::internal::CaptureStderr();

        std::string operation_error;

        try {
            std::forward<Operation>(operation)();
        } catch (const std::exception &exception) {
            operation_error = exception.what();
        } catch (...) {
            operation_error = "Unknown exception";
        }

        const std::string validation_output =
            testing::internal::GetCapturedStderr();

        return {validation_output, operation_error};
    }

    [[nodiscard]] auto
    hasUnexpectedSwapchainValidationOutput(std::string_view output) -> bool {
        constexpr std::string_view kDiagnosticPrefix = "Vulkan validation:";

        constexpr std::string_view kSkippedDriverPrefix =
            " terminator_CreateInstance: Received return code ";

        constexpr std::string_view kSkippedDriverSuffix =
            "Skipping this driver.";

        std::size_t position{};

        while ((position = output.find(kDiagnosticPrefix, position)) !=
               std::string_view::npos) {
            const std::size_t message_start =
                position + kDiagnosticPrefix.size();

            const std::size_t message_end = output.find('\n', message_start);

            const std::string_view message = output.substr(
                message_start, message_end == std::string_view::npos
                                   ? std::string_view::npos
                                   : message_end - message_start);

            if (!(message.starts_with(kSkippedDriverPrefix) &&
                  message.ends_with(kSkippedDriverSuffix))) {
                return true;
            }

            if (message_end == std::string_view::npos) {
                break;
            }

            position = message_end + 1U;
        }

        return false;
    }
} // namespace

class VulkanSwapchainTransferSourceIntegrationTests : public ::testing::Test {
  protected:
    std::optional<Platform::GlfwContext> m_Glfw;
    std::optional<Platform::Window> m_Window;
    std::optional<Vulkan::VulkanInstance> m_Instance;
    std::optional<Vulkan::VulkanSurface> m_Surface;
    std::optional<Vulkan::SelectedPhysicalDevice> m_SelectedDevice;
    std::optional<Vulkan::VulkanDevice> m_Device;

    auto SetUp() -> void override {
        if (std::getenv("DISPLAY") == nullptr &&
            std::getenv("WAYLAND_DISPLAY") == nullptr) {
            GTEST_SKIP() << "Requires an available window-system display";
        }

        m_Glfw.emplace();

        constexpr int width = 640;
        constexpr int height = 480;

        m_Window.emplace(Platform::WindowSize{.width = width, .height = height},
                         "Signum Swapchain Transfer Source Test");

        m_Instance.emplace("Signum Swapchain Transfer Source Test", true);

        m_Surface.emplace(m_Instance->nativeHandle(), m_Window->nativeHandle());

        const auto available_devices =
            Vulkan::enumeratePhysicalDevices(m_Instance->nativeHandle());

        const auto discovered_devices = Vulkan::inspectPhysicalDevices(
            available_devices, m_Surface->nativeHandle());

        m_SelectedDevice = Vulkan::selectPhysicalDevice(discovered_devices);

        if (!m_SelectedDevice.has_value()) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }

        const auto feature_request = Vulkan::deriveLogicalDeviceFeatureRequest(
            Renderer::PresentationPreference::VSync);

        const auto feature_configuration =
            Vulkan::deriveLogicalDeviceFeatureConfiguration(
                feature_request, m_SelectedDevice->capabilities);

        const auto queue_requests = Vulkan::deriveUniqueQueueFamilyRequests(
            m_SelectedDevice->queue_families);

        m_Device.emplace(m_SelectedDevice->handle,
                         m_SelectedDevice->queue_families, queue_requests,
                         feature_configuration,
                         m_SelectedDevice->capabilities.optional_capabilities);
    }

    [[nodiscard]] auto supportsTransferSource() const -> bool {
        if (!m_SelectedDevice.has_value() || !m_Surface.has_value()) {
            throw std::logic_error{
                "Swapchain transfer-source test requires an initialized "
                "physical device and surface"};
        }

        const Vulkan::SwapchainSupportDetails support =
            Vulkan::querySwapchainSupport(m_SelectedDevice->handle,
                                          m_Surface->nativeHandle());

        return (support.surface_capabilities.supportedUsageFlags &
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0U;
    }

    [[nodiscard]] auto createSwapchain(bool require_transfer_source)
        -> Vulkan::VulkanSwapchain {
        if (!m_SelectedDevice.has_value() || !m_Device.has_value() ||
            !m_Surface.has_value() || !m_Window.has_value()) {
            throw std::logic_error{
                "Swapchain transfer-source test requires initialized "
                "Vulkan and window resources"};
        }

        return Vulkan::VulkanSwapchain{
            m_SelectedDevice->handle,
            m_Device->nativeHandle(),
            m_Surface->nativeHandle(),
            m_SelectedDevice->queue_families,
            m_Window->framebufferSize(),
            Renderer::PresentationPreference::VSync,
            false,
            VK_NULL_HANDLE,
            require_transfer_source,
        };
    }
};

TEST(VulkanSwapchainRetirementIntegrationTests,
     ClearsRetiredSwapchainAfterReplacementConstructionFails) {
    if (std::getenv("DISPLAY") == nullptr &&
        std::getenv("WAYLAND_DISPLAY") == nullptr) {
        GTEST_SKIP() << "Requires an available window-system display";
    }

    Platform::GlfwContext glfw{};

    const std::uint32_t width = 640;
    const std::uint32_t height = 480;

    const Platform::Window window{
        {.width = width, .height = height},
        "Signum Swapchain Retirement Test",
    };

    Vulkan::VulkanInstance instance{"Signum Swapchain Test", false};

    Vulkan::VulkanSurface surface{
        instance.nativeHandle(),
        window.nativeHandle(),
    };

    const auto available_devices =
        Vulkan::enumeratePhysicalDevices(instance.nativeHandle());

    const auto discovered_devices = Vulkan::inspectPhysicalDevices(
        available_devices, surface.nativeHandle());

    const auto selected_device =
        Vulkan::selectPhysicalDevice(discovered_devices);

    if (!selected_device.has_value()) {
        GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
    }

    const auto feature_request = Vulkan::deriveLogicalDeviceFeatureRequest(
        Renderer::PresentationPreference::VSync);

    const auto feature_configuration =
        Vulkan::deriveLogicalDeviceFeatureConfiguration(
            feature_request, selected_device->capabilities);

    const auto queue_requests = Vulkan::deriveUniqueQueueFamilyRequests(
        selected_device->queue_families);

    Vulkan::VulkanDevice device{
        selected_device->handle,
        selected_device->queue_families,
        queue_requests,
        feature_configuration,
        selected_device->capabilities.optional_capabilities,
    };

    const auto make_swapchain =
        [&](VkSwapchainKHR old_swapchain) -> Vulkan::VulkanSwapchain {
        return Vulkan::VulkanSwapchain{
            selected_device->handle,
            device.nativeHandle(),
            surface.nativeHandle(),
            selected_device->queue_families,
            window.framebufferSize(),
            Renderer::PresentationPreference::VSync,
            false,
            old_swapchain,
        };
    };

    std::optional<Vulkan::VulkanSwapchain> current;

    current.emplace(make_swapchain(VK_NULL_HANDLE));

    ASSERT_TRUE(current.has_value());
    ASSERT_NE(current->nativeHandle(), VK_NULL_HANDLE);

    const VkSwapchainKHR old_handle = current->nativeHandle();

    bool replacement_created = false;

    std::ostringstream diagnostics;

    const int exit_status = Core::Error::runWithErrorBoundary(
        [&] -> void {
            Vulkan::replaceRetiringResource(
                current, [&] -> Vulkan::VulkanSwapchain {
                    Vulkan::VulkanSwapchain replacement =
                        make_swapchain(old_handle);

                    replacement_created =
                        replacement.nativeHandle() != VK_NULL_HANDLE;

                    throw std::runtime_error{
                        "Deliberate failure after replacement creation"};
                });
        },
        diagnostics);

    EXPECT_EQ(exit_status, EXIT_FAILURE);

    EXPECT_EQ(diagnostics.str(),
              "Unexpected standard exception: "
              "Deliberate failure after replacement creation\n");

    EXPECT_TRUE(replacement_created);
    EXPECT_FALSE(current.has_value());

    device.waitIdle();
}

TEST_F(VulkanSwapchainTransferSourceIntegrationTests,
       CreatesTransferSourceEnabledSwapchainWhenSupported) {
    if (!supportsTransferSource()) {
        GTEST_SKIP()
            << "Surface does not support transfer-source swapchain images";
    }

    const auto [validation_output, operation_error] =
        captureSwapchainValidationOutput([&] -> void {
            Vulkan::VulkanSwapchain swapchain = createSwapchain(true);

            EXPECT_NE(swapchain.nativeHandle(), VK_NULL_HANDLE);

            EXPECT_FALSE(swapchain.images().empty());

            EXPECT_EQ(swapchain.imageViews().size(), swapchain.images().size());

            for (const VkImage image : swapchain.images()) {
                EXPECT_NE(image, VK_NULL_HANDLE);
            }

            for (const VkImageView image_view : swapchain.imageViews()) {
                EXPECT_NE(image_view, VK_NULL_HANDLE);
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Transfer-source swapchain creation failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedSwapchainValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanSwapchainTransferSourceIntegrationTests,
       RejectsTransferSourceWhenUnsupported) {
    if (supportsTransferSource()) {
        GTEST_SKIP() << "Surface supports transfer-source swapchain images";
    }

    bool rejected = false;

    const auto [validation_output, operation_error] =
        captureSwapchainValidationOutput([&] -> void {
            try {
                [[maybe_unused]]
                Vulkan::VulkanSwapchain swapchain = createSwapchain(true);
            } catch (const Core::Error::EngineError &error) {
                rejected = true;

                EXPECT_EQ(error.getCode(),
                          Core::Error::Code::VulkanSwapchainCreationFailed);
            }
        });

    EXPECT_TRUE(rejected)
        << "Unsupported transfer-source usage must be rejected";

    EXPECT_TRUE(operation_error.empty())
        << "Unexpected exception: " << operation_error;

    EXPECT_FALSE(hasUnexpectedSwapchainValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanSwapchainTransferSourceIntegrationTests,
       RendererRendersWithTransferSourceRequested) {
    if (!supportsTransferSource()) {
        GTEST_SKIP()
            << "Surface does not support transfer-source swapchain images";
    }

    if (!m_Window.has_value()) {
        FAIL() << "Transfer-source renderer test requires an initialized "
                  "window";
    }

    const Platform::Window &window = m_Window.value();

    const auto [validation_output, operation_error] =
        captureSwapchainValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Transfer Source Renderer Test",
                window,
                Renderer::PresentationPreference::VSync,
                true,
                std::nullopt,
                std::nullopt,
                std::nullopt,
                std::nullopt,
                {},
                {},
                true,
            };

            constexpr std::size_t kFrameCount{4U};

            for (std::size_t frame{}; frame < kFrameCount; ++frame) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Transfer-source renderer initialization or rendering failed: "
        << operation_error;

    EXPECT_FALSE(hasUnexpectedSwapchainValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}
