#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/log_record.hpp"
#include "engine/core/logging/log_sink.hpp"
#include "engine/core/logging/logger.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Logging = SNE::Engine::Core::Logging;
namespace Platform = SNE::Engine::Platform;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    class CapturingSink final : public Logging::LogSink {
      private:
        std::vector<Logging::LogRecord> &m_Records;

      public:
        explicit CapturingSink(std::vector<Logging::LogRecord> &records)
            : m_Records(records) {}

        auto write(const Logging::LogRecord &record) -> void override {
            m_Records.push_back(record);
        }
    };

    class ThrowingSink final : public Logging::LogSink {
      private:
        std::atomic<bool> &m_ReceivedMessage;

      public:
        explicit ThrowingSink(std::atomic<bool> &received_message)
            : m_ReceivedMessage(received_message) {}

        auto write(const Logging::LogRecord &record) -> void override {
            if (record.message.contains("Signum injected sink failure")) {
                m_ReceivedMessage.store(true);
            }

            throw std::runtime_error{"Injected Vulkan logging failure"};
        }
    };

    [[nodiscard]] auto canRunValidationTest() -> bool {
        if (std::getenv("DISPLAY") == nullptr &&
            std::getenv("WAYLAND_DISPLAY") == nullptr) {
            return false;
        }

        std::uint32_t count{};
        if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS) {
            return false;
        }

        std::vector<VkLayerProperties> layers(count);
        if (vkEnumerateInstanceLayerProperties(&count, layers.data()) !=
            VK_SUCCESS) {
            return false;
        }

        return std::ranges::any_of(layers, [](const auto &layer) -> bool {
            return std::strcmp(layer.layerName,
                               "VK_LAYER_KHRONOS_validation") == 0;
        });
    }

    [[nodiscard]] auto submitFunction(VkInstance instance)
        -> PFN_vkSubmitDebugUtilsMessageEXT {
        return reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(
            vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT"));
    }

    auto submitDiagnostic(PFN_vkSubmitDebugUtilsMessageEXT submit,
                          VkInstance instance,
                          VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                          const char *message) -> void {
        VkDebugUtilsMessengerCallbackDataEXT data{};
        data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
        data.pMessage = message;

        submit(instance, severity, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
               &data);
    }
} // namespace

TEST(VulkanInstanceLoggingIntegrationTests,
     RoutesWarningAndErrorDiagnosticsToLogger) {
    if (!canRunValidationTest()) {
        GTEST_SKIP() << "Requires a display and Vulkan validation layer";
    }

    Platform::GlfwContext glfw{};

    std::vector<Logging::LogRecord> records;
    Logging::Logger logger{std::make_unique<CapturingSink>(records)};

    Vulkan::VulkanInstance instance{"Signum Vulkan Logging Integration Test",
                                    true, &logger};

    const auto submit = submitFunction(instance.nativeHandle());
    ASSERT_NE(submit, nullptr);

    submitDiagnostic(submit, instance.nativeHandle(),
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                     "Signum injected warning");

    submitDiagnostic(submit, instance.nativeHandle(),
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                     "Signum injected error");

    const auto warning = std::ranges::find_if(
        records, [](const Logging::LogRecord &record) -> bool {
            return record.message.contains("Signum injected warning");
        });

    const auto error = std::ranges::find_if(
        records, [](const Logging::LogRecord &record) -> bool {
            return record.message.contains("Signum injected error");
        });

    ASSERT_NE(warning, records.end());
    ASSERT_NE(error, records.end());

    EXPECT_EQ(warning->level, Logging::LogLevel::Warning);
    EXPECT_EQ(warning->category, "Vulkan");

    EXPECT_EQ(error->level, Logging::LogLevel::Error);
    EXPECT_EQ(error->category, "Vulkan");
}

TEST(VulkanInstanceLoggingIntegrationTests,
     LoggingFailureDoesNotEscapeVulkanCallback) {
    if (!canRunValidationTest()) {
        GTEST_SKIP() << "Requires a display and Vulkan validation layer";
    }

    Platform::GlfwContext glfw{};

    std::atomic<bool> received_message{false};

    Logging::Logger logger{std::make_unique<ThrowingSink>(received_message)};

    Vulkan::VulkanInstance instance{"Signum Vulkan Logging Failure Test", true,
                                    &logger};

    const auto submit = submitFunction(instance.nativeHandle());
    ASSERT_NE(submit, nullptr);

    EXPECT_NO_THROW(
        submitDiagnostic(submit, instance.nativeHandle(),
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                         "Signum injected sink failure"));

    EXPECT_TRUE(received_message.load());
}
