#include "vulkan_renderer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/common/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain_result_policy.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_semaphore.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;
namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr std::size_t kMatrixSize = 64U;
    constexpr std::size_t kMatrixOffset = 0U;

    static_assert(sizeof(Math::Matrix4f) == kMatrixSize);
    static_assert(offsetof(Math::Matrix4f, elements) == kMatrixOffset);
    static_assert(std::is_trivially_copyable_v<Math::Matrix4f>);

    struct QuadVertex {
        std::array<float, 2> position;
        std::array<float, 3> color;
    };
    constexpr float kQuadScale = 0.5F;
    constexpr float kQuadHorizontalOffset = 0.5F;
    constexpr float kQuadRotationDegrees = 45.0F;
    constexpr float kDegreesPerHalfTurn = 180.0F;

    constexpr float kQuadRotationRadians =
        kQuadRotationDegrees * std::numbers::pi_v<float> / kDegreesPerHalfTurn;

    constexpr std::array<QuadVertex, 4> kQuadVertices{
        {
            {
                // bottom-left
                .position = {-0.5F, -0.5F},
                .color = {1.0F, 0.0F, 0.0F},
            },
            {
                // bottom-right
                .position = {0.5F, -0.5F},
                .color = {0.0F, 1.0F, 0.0F},
            },
            {
                // top-left
                .position = {-0.5F, 0.5F},
                .color = {0.0F, 0.0F, 1.0F},
            },
            {
                // top-right
                .position = {0.5F, 0.5F},
                .color = {0.0F, 0.0F, 1.0F},
            },
        },
    };

    constexpr std::array<std::uint16_t, 6> kQuadIndices{0, 1, 2, 2, 1, 3};
    static_assert(kQuadIndices.size() <=
                      std::numeric_limits<std::uint32_t>::max(),
                  "Quad Indices size exceeds uint32_t");

    static_assert(sizeof(QuadVertex) <=
                      std::numeric_limits<std::uint32_t>::max(),
                  "QuadVertex size exceeds uint32_t");

    [[nodiscard]] auto selectRequiredPhysicalDevice(VkInstance instance,
                                                    VkSurfaceKHR surface)
        -> Vulkan::SelectedPhysicalDevice {
        const std::vector<VkPhysicalDevice> available_devices =
            Vulkan::enumeratePhysicalDevices(instance);

        const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices =
            Vulkan::inspectPhysicalDevices(available_devices, surface);

        std::optional<Vulkan::SelectedPhysicalDevice> selected_device =
            Vulkan::selectPhysicalDevice(discovered_devices);

        if (!selected_device.has_value()) {
            throw Error::EngineError(
                Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
                "No suitable Vulkan physical device is available",
                "Obtain Vulkan Suitable Physical Device");
        }

        return selected_device.value();
    }

    constexpr std::size_t kFramesInFlight = 2;
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    VulkanRenderer::VulkanRenderer(
        const std::string &application_name, const Platform::Window &window,
        PresentationPreference presentation_preference,
        bool development_diagnostics_enabled)
        : m_Window(&window),
          m_Instance(application_name, development_diagnostics_enabled),
          m_Surface(m_Instance.nativeHandle(), window.nativeHandle()),
          m_PhysicalDevice(selectRequiredPhysicalDevice(
              m_Instance.nativeHandle(), m_Surface.nativeHandle())),
          m_PresentationPreference(presentation_preference),
          m_LogicalDeviceFeatureConfiguration(
              deriveLogicalDeviceFeatureConfiguration(
                  deriveLogicalDeviceFeatureRequest(presentation_preference),
                  m_PhysicalDevice.capabilities)),
          m_Device(
              m_PhysicalDevice.handle, m_PhysicalDevice.queue_families,
              deriveUniqueQueueFamilyRequests(m_PhysicalDevice.queue_families),
              m_LogicalDeviceFeatureConfiguration,
              m_PhysicalDevice.capabilities.optional_capabilities),
          m_ImmediateSubmission(
              m_Device.nativeHandle(), m_Device.graphicsQueue(),
              m_PhysicalDevice.queue_families.graphics_family.family_index),
          m_MemoryAllocator(m_Instance.nativeHandle(), m_PhysicalDevice.handle,
                            m_Device.nativeHandle(), kRequiredApiVersion,
                            m_PhysicalDevice.capabilities.optional_capabilities
                                .memory_budget_extension_supported) {
        const std::uint32_t graphics_queue_family_index =
            m_PhysicalDevice.queue_families.graphics_family.family_index;

        m_FrameResources.reserve(kFramesInFlight);

        for (std::size_t i{}; i < kFramesInFlight; ++i) {
            m_FrameResources.emplace_back(m_Device.nativeHandle(),
                                          graphics_queue_family_index);
        }

        VulkanBufferCreateInfo buffer_create_info{
            .size = static_cast<VkDeviceSize>(sizeof(kQuadVertices)),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .memory_usage = GpuMemoryUsage::Device,
        };

        m_QuadVertexBuffer.emplace(m_MemoryAllocator, buffer_create_info);

        const auto vertex_bytes = std::as_bytes(std::span{kQuadVertices});
        uploadBufferData(m_MemoryAllocator, m_ImmediateSubmission,
                         m_QuadVertexBuffer.value(), vertex_bytes);

        VulkanBufferCreateInfo index_buffer_create_info{
            .size = static_cast<VkDeviceSize>(sizeof(kQuadIndices)),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            .memory_usage = GpuMemoryUsage::Device,
        };

        m_QuadIndexBuffer.emplace(m_MemoryAllocator, index_buffer_create_info);
        const auto index_bytes = std::as_bytes(std::span{kQuadIndices});
        uploadBufferData(m_MemoryAllocator, m_ImmediateSubmission,
                         m_QuadIndexBuffer.value(), index_bytes);

        if (isFramebufferDrawable()) {
            static_cast<void>(recreateSwapchainResources());
        }
    }

    VulkanRenderer::~VulkanRenderer() noexcept {
        const VkResult result = vkDeviceWaitIdle(m_Device.nativeHandle());
        static_cast<void>(result);
    }

    auto VulkanRenderer::waitForDrawableFramebuffer() const -> bool {
        while (!isFramebufferDrawable() && !m_Window->shouldClose()) {
            Platform::Window::waitEvents();
        }

        return isFramebufferDrawable();
    }

    auto VulkanRenderer::isFramebufferDrawable() const -> bool {
        const Platform::FramebufferSize framebuffer_size =
            m_Window->framebufferSize();
        return framebuffer_size.width > 0 && framebuffer_size.height > 0;
    }

    auto VulkanRenderer::recreateSwapchainResources() -> bool {
        if (!waitForDrawableFramebuffer()) {
            return false;
        }

        m_Device.waitIdle();

        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
        if (m_SwapchainResources.has_value()) {
            old_swapchain =
                m_SwapchainResources.value().m_VulkanSwapchain.nativeHandle();
        }

        const Platform::FramebufferSize new_framebuffer_size =
            m_Window->framebufferSize();

        VulkanSwapchain new_swapchain = VulkanSwapchain(
            m_PhysicalDevice.handle, m_Device.nativeHandle(),
            m_Surface.nativeHandle(), m_PhysicalDevice.queue_families,
            new_framebuffer_size, m_PresentationPreference,
            m_LogicalDeviceFeatureConfiguration.fifo_latest_ready_feature
                    .presentModeFifoLatestReady == VK_TRUE,
            old_swapchain);

        std::optional<VulkanGraphicsPipeline> new_graphics_pipeline{};

        if (!m_SwapchainResources.has_value() ||
            new_swapchain.surfaceFormat().format !=
                m_SwapchainResources.value()
                    .m_VulkanSwapchain.surfaceFormat()
                    .format) {
            const auto fragment_data =
                loadSpirv("build/shaders/triangle.frag.spv");
            const auto vertex_data =
                loadSpirv("build/shaders/triangle.vert.spv");
            const GraphicsShaderData graphics_data{
                .vertex =
                    {
                        .words = vertex_data.words,
                        .selection =
                            {
                                .stage = Shader::ShaderStage::Vertex,
                                .entry_point = "main",
                            },
                    },
                .fragment =
                    {
                        .words = fragment_data.words,
                        .selection =
                            {
                                .stage = Shader::ShaderStage::Fragment,
                                .entry_point = "main",
                            },
                    },
            };

            VkVertexInputBindingDescription vertex_binding{};
            vertex_binding.binding = 0U;
            vertex_binding.stride =
                static_cast<std::uint32_t>(sizeof(QuadVertex));
            vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            VkVertexInputAttributeDescription position_attribute{};
            position_attribute.location = 0U;
            position_attribute.binding = 0U;
            position_attribute.format = VK_FORMAT_R32G32_SFLOAT;
            position_attribute.offset =
                static_cast<std::uint32_t>(offsetof(QuadVertex, position));

            VkVertexInputAttributeDescription color_attribute{};
            color_attribute.location = 1U;
            color_attribute.binding = 0U;
            color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            color_attribute.offset =
                static_cast<std::uint32_t>(offsetof(QuadVertex, color));

            const std::array<VkVertexInputBindingDescription, 1>
                vertex_bindings{vertex_binding};

            const std::array<VkVertexInputAttributeDescription, 2>
                vertex_attributes{
                    position_attribute,
                    color_attribute,
            };

            const GraphicsVertexInputData vertex_input_data{
                .bindings = vertex_bindings,
                .attributes = vertex_attributes,
            };

            const std::array<VkPushConstantRange, 1> push_constant_ranges{
                {
                    {
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                        .offset = 0U,
                        .size =
                            static_cast<std::uint32_t>(sizeof(Math::Matrix4f)),
                    },
                },
            };

            const GraphicsPipelineLayoutData graphics_pipeline_layout_data{
                .push_constant_ranges = push_constant_ranges,
            };

            new_graphics_pipeline.emplace(m_Device.nativeHandle(),
                                          new_swapchain.surfaceFormat().format,
                                          graphics_data, vertex_input_data,
                                          graphics_pipeline_layout_data);
        }

        std::vector<VulkanSemaphore> new_render_finished_semaphores;
        new_render_finished_semaphores.reserve(new_swapchain.images().size());

        for (std::size_t i{}; i < new_swapchain.images().size(); ++i) {
            new_render_finished_semaphores.emplace_back(
                m_Device.nativeHandle());
        }

        if (!new_graphics_pipeline.has_value()) {
            new_graphics_pipeline.emplace(
                std::move(m_SwapchainResources->m_VulkanGraphicsPipeline));
        }

        m_SwapchainResources.emplace(SwapchainResources{
            .m_VulkanSwapchain = std::move(new_swapchain),
            .m_VulkanGraphicsPipeline =
                std::move(new_graphics_pipeline.value()),
            .m_VulkanSemaphores = std::move(new_render_finished_semaphores),
        });

        return true;
    }

    auto VulkanRenderer::renderFrame() -> void {
        if (!m_SwapchainResources.has_value() &&
            !recreateSwapchainResources()) {
            return;
        }

        if (!m_SwapchainResources.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires swapchain resources after successful "
                "creation");
        }

        SwapchainResources &swapchain_resources = m_SwapchainResources.value();

        VulkanFrameResources &current_frame =
            m_FrameResources[m_CurrentFrameIndex];

        current_frame.inFlightFence().wait();

        std::uint32_t image_index{};

        const VkSwapchainKHR swapchain_handle =
            swapchain_resources.m_VulkanSwapchain.nativeHandle();

        const VulkanSwapchain &swapchain =
            swapchain_resources.m_VulkanSwapchain;

        const VulkanGraphicsPipeline &graphics_pipeline =
            swapchain_resources.m_VulkanGraphicsPipeline;

        const auto &semaphores = swapchain_resources.m_VulkanSemaphores;

        const VkResult image_result = vkAcquireNextImageKHR(
            m_Device.nativeHandle(), swapchain_handle, UINT32_MAX,
            current_frame.imageAvailableSemaphore().nativeHandle(),
            VK_NULL_HANDLE, &image_index);

        bool swapchain_recreation_requested = false;

        const SwapchainAcquireAction acquire_action =
            selectSwapchainAcquireAction(image_result);

        switch (acquire_action) {
        case SwapchainAcquireAction::ContinueFrame:
            break;

        case SwapchainAcquireAction::ContinueFrameAndRecreateAfterPresent:
            swapchain_recreation_requested = true;
            break;

        case SwapchainAcquireAction::ReturnWithoutAdvancingFrame:
            return;

        case SwapchainAcquireAction::RecreateAndReturnWithoutAdvancingFrame:
            static_cast<void>(recreateSwapchainResources());
            return;

        case SwapchainAcquireAction::Failure:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainImageAcquisitionFailed,
                "Failed to acquire Vulkan swapchain image",
                Core::Error::NativeError(static_cast<int>(image_result),
                                         std::string(toString(image_result))),
                "Acquire Vulkan Swapchain Image");
        }

        if (swapchain.images().size() != swapchain.imageViews().size() ||
            swapchain.images().size() != semaphores.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires swapchain images, image views, and "
                "render-finished semaphores to have matching counts");
        }

        const auto acquired_image_index = static_cast<std::size_t>(image_index);

        if (acquired_image_index >= swapchain.images().size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer acquired a swapchain image index outside the "
                "tracked swapchain image range");
        }

        current_frame.resetCommandResources();

        if (current_frame.commandBuffers().empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires the current frame to contain at least "
                "one command buffer");
        }

        const VkCommandBuffer command_buffer =
            current_frame.commandBuffers().front();

        VkCommandBufferBeginInfo command_buffer_info{};
        command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        const VkResult command_buffer_begin_result =
            vkBeginCommandBuffer(command_buffer, &command_buffer_info);

        if (command_buffer_begin_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferBeginFailed,
                "Failed to begin Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_begin_result),
                    std::string(toString(command_buffer_begin_result))),
                "Begin Vulkan Command Buffer");
        }

        const VkImage acquired_image = swapchain.images()[acquired_image_index];

        VkImageMemoryBarrier2 color_attachment_barrier{};
        color_attachment_barrier.sType =
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        color_attachment_barrier.image = acquired_image;
        color_attachment_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_barrier.newLayout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        color_attachment_barrier.srcAccessMask = VK_ACCESS_2_NONE;
        color_attachment_barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        color_attachment_barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        color_attachment_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        color_attachment_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        color_attachment_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VkDependencyInfo color_attachment_dependency_info{};
        color_attachment_dependency_info.sType =
            VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        color_attachment_dependency_info.imageMemoryBarrierCount = 1U;
        color_attachment_dependency_info.pImageMemoryBarriers =
            &color_attachment_barrier;

        vkCmdPipelineBarrier2(command_buffer,
                              &color_attachment_dependency_info);

        const VkImageView acquired_image_view =
            swapchain.imageViews()[acquired_image_index];

        VkRenderingAttachmentInfo render_attachment_info{};
        render_attachment_info.sType =
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        render_attachment_info.imageView = acquired_image_view;
        render_attachment_info.imageLayout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        const VkClearColorValue color{{0.0F, 0.0F, 0.0F, 1.0F}};

        render_attachment_info.clearValue = {
            .color = color,
        };

        VkRenderingInfo render_info{};
        render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        render_info.renderArea = {
            .offset = {.x = 0, .y = 0},
            .extent = swapchain.extent(),
        };
        render_info.layerCount = 1U;
        render_info.colorAttachmentCount = 1U;
        render_info.pColorAttachments = &render_attachment_info;

        vkCmdBeginRendering(command_buffer, &render_info);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphics_pipeline.nativeHandle());

        const VkDeviceSize vertex_offset = 0U;
        if (!m_QuadVertexBuffer.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires an initialized vertex buffer before "
                "recording draw commands");
        }
        const VkBuffer vertex_buffer =
            m_QuadVertexBuffer.value().nativeHandle();
        vkCmdBindVertexBuffers(command_buffer, 0U, 1U, &vertex_buffer,
                               &vertex_offset);

        if (!m_QuadIndexBuffer.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires an initialized index buffer before "
                "recording indexed draw commands");
        }
        const VkBuffer index_buffer = m_QuadIndexBuffer.value().nativeHandle();
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0U,
                             VK_INDEX_TYPE_UINT16);

        VkViewport viewport{};
        viewport.x = 0.0F;
        viewport.y = static_cast<float>(swapchain.extent().height);
        viewport.width = static_cast<float>(swapchain.extent().width);
        viewport.height = -static_cast<float>(swapchain.extent().height);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;
        vkCmdSetViewport(command_buffer, 0U, 1U, &viewport);

        VkRect2D scissor{};
        scissor.extent = swapchain.extent();
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(command_buffer, 0U, 1U, &scissor);

        const auto indices = static_cast<std::uint32_t>(kQuadIndices.size());
        const Math::Vector3f translation_a{
            .x = -kQuadHorizontalOffset,
            .y = 0.0F,
            .z = 0.0F,
        };

        const Math::Vector3f rotation_a{
            .x = 0.0F,
            .y = 0.0F,
            .z = kQuadRotationRadians,
        };

        const Math::Vector3f scale_a{
            .x = kQuadScale,
            .y = kQuadScale,
            .z = 1.0F,
        };

        const Math::Matrix4f model_a =
            Math::composeTRS(translation_a, rotation_a, scale_a);

        vkCmdPushConstants(command_buffer, graphics_pipeline.layoutHandle(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0U,
                           static_cast<std::uint32_t>(sizeof(model_a)),
                           &model_a);
        vkCmdDrawIndexed(command_buffer, indices, 1U, 0U, 0, 0U);

        const Math::Vector3f translation_b{
            .x = kQuadHorizontalOffset,
            .y = 0.0F,
            .z = 0.0F,
        };

        const Math::Vector3f rotation_b{};

        const Math::Matrix4f model_b =
            Math::composeTRS(translation_b, rotation_b, scale_a);

        vkCmdPushConstants(command_buffer, graphics_pipeline.layoutHandle(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0U,
                           static_cast<std::uint32_t>(sizeof(model_b)),
                           &model_b);
        vkCmdDrawIndexed(command_buffer, indices, 1U, 0U, 0, 0U);

        vkCmdEndRendering(command_buffer);

        VkImageMemoryBarrier2 present_barrier{};
        present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        present_barrier.image = acquired_image;
        present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        present_barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        present_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        present_barrier.dstAccessMask = VK_ACCESS_2_NONE;
        present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        present_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VkDependencyInfo present_dependency_info{};
        present_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        present_dependency_info.imageMemoryBarrierCount = 1U;
        present_dependency_info.pImageMemoryBarriers = &present_barrier;

        vkCmdPipelineBarrier2(command_buffer, &present_dependency_info);

        const VkResult command_buffer_end_result =
            vkEndCommandBuffer(command_buffer);

        if (command_buffer_end_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferEndFailed,
                "Failed to end Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_end_result),
                    std::string(toString(command_buffer_end_result))),
                "End Vulkan Command Buffer");
        }

        const VulkanSemaphore &render_finished_semaphore =
            semaphores[acquired_image_index];

        const VkSemaphore render_finished_handle =
            render_finished_semaphore.nativeHandle();

        VkSemaphoreSubmitInfo image_available_wait_info{};
        image_available_wait_info.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        image_available_wait_info.semaphore =
            current_frame.imageAvailableSemaphore().nativeHandle();
        image_available_wait_info.value = std::uint64_t{0};
        image_available_wait_info.stageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_wait_info.deviceIndex = 0U;

        VkCommandBufferSubmitInfo command_buffer_submit_info{};
        command_buffer_submit_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = command_buffer;
        command_buffer_submit_info.deviceMask = 0U;

        VkSemaphoreSubmitInfo render_finished_signal_info{};
        render_finished_signal_info.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        render_finished_signal_info.semaphore = render_finished_handle;
        render_finished_signal_info.value = std::uint64_t{0};
        render_finished_signal_info.stageMask =
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        render_finished_signal_info.deviceIndex = 0U;

        VkSubmitInfo2 submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.waitSemaphoreInfoCount = 1U;
        submit_info.pWaitSemaphoreInfos = &image_available_wait_info;
        submit_info.commandBufferInfoCount = 1U;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;
        submit_info.signalSemaphoreInfoCount = 1U;
        submit_info.pSignalSemaphoreInfos = &render_finished_signal_info;

        current_frame.inFlightFence().reset();

        const VkResult queue_submit_result =
            vkQueueSubmit2(m_Device.graphicsQueue(), 1U, &submit_info,
                           current_frame.inFlightFence().nativeHandle());

        if (queue_submit_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueueSubmissionFailed,
                "Failed to submit Vulkan queue work",
                Core::Error::NativeError(
                    static_cast<int>(queue_submit_result),
                    std::string(toString(queue_submit_result))),
                "Submit Vulkan Queue Work");
        }

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1U;
        present_info.pWaitSemaphores = &render_finished_handle;
        present_info.swapchainCount = 1U;
        present_info.pSwapchains = &swapchain_handle;
        present_info.pImageIndices = &image_index;

        const VkResult queue_present_result =
            vkQueuePresentKHR(m_Device.presentationQueue(), &present_info);

        const SwapchainPresentAction present_action =
            selectSwapchainPresentAction(queue_present_result);

        switch (present_action) {
        case SwapchainPresentAction::AdvanceFrame:
            break;

        case SwapchainPresentAction::RecreateAndAdvanceFrame:
            swapchain_recreation_requested = true;
            break;

        case SwapchainPresentAction::RecreateAndReturnWithoutAdvancingFrame:
            static_cast<void>(recreateSwapchainResources());
            return;

        case SwapchainPresentAction::Failure:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueuePresentationFailed,
                "Failed to present Vulkan swapchain image",
                Core::Error::NativeError(
                    static_cast<int>(queue_present_result),
                    std::string(toString(queue_present_result))),
                "Present Vulkan Swapchain Image");
        }

        if (swapchain_recreation_requested) {
            static_cast<void>(recreateSwapchainResources());
        }

        m_CurrentFrameIndex =
            (m_CurrentFrameIndex + 1U) % m_FrameResources.size();
    }
} // namespace SNE::Engine::Renderer::Vulkan
