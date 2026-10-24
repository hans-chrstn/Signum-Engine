#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include "engine/renderer/vulkan/geometry/vulkan_vertex_input.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_draw_push_constants.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include "vulkan_renderer.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace Math = SNE::Engine::Core::Math;

namespace SNE::Engine::Renderer::Vulkan {
    auto VulkanRenderer::isFramebufferDrawable() const -> bool {
        const Platform::FramebufferSize framebuffer_size =
            m_Window->framebufferSize();
        return framebuffer_size.width > 0 && framebuffer_size.height > 0;
    }

    auto VulkanRenderer::waitForDrawableFramebuffer() const -> bool {
        while (!isFramebufferDrawable() && !m_Window->shouldClose()) {
            Platform::Window::waitEvents();
        }

        return isFramebufferDrawable();
    }

    auto VulkanRenderer::createDepthAttachments(VkExtent2D extent,
                                                VkFormat depth_format)
        -> std::vector<DepthAttachmentResources> {
        const VulkanImageCreateInfo depth_image_create_info{
            .extent =
                {
                    .width = extent.width,
                    .height = extent.height,
                    .depth = 1U,
                },
            .image_type = VK_IMAGE_TYPE_2D,
            .format = depth_format,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        };

        std::vector<DepthAttachmentResources> new_depth_attachments;
        new_depth_attachments.reserve(m_FrameResources.size());

        for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
            VulkanImage depth_image{m_MemoryAllocator, depth_image_create_info};

            VkImageViewCreateInfo depth_view_create_info{};
            depth_view_create_info.sType =
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            depth_view_create_info.image = depth_image.nativeHandle();
            depth_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depth_view_create_info.format = depth_format;
            depth_view_create_info.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_DEPTH_BIT;
            depth_view_create_info.subresourceRange.baseMipLevel = 0U;
            depth_view_create_info.subresourceRange.levelCount = 1U;
            depth_view_create_info.subresourceRange.baseArrayLayer = 0U;
            depth_view_create_info.subresourceRange.layerCount = 1U;

            VulkanImageView depth_image_view{m_Device.nativeHandle(),
                                             depth_view_create_info};

            new_depth_attachments.emplace_back(DepthAttachmentResources{
                .m_Image = std::move(depth_image),
                .m_ImageView = std::move(depth_image_view),
            });
        }
        return new_depth_attachments;
    }

    auto VulkanRenderer::getOrCreateGraphicsPipeline(
        const MaterialDescription &material, const VulkanSwapchain &swapchain,
        VkFormat depth_format) -> const VulkanGraphicsPipeline * {
        const Shader::GraphicsShaderData &graphics_data = material.shaders;

        const VertexLayoutDescription vertex_layout{
            .attributes = m_MeshVertexAttributes,
        };

        const VulkanVertexInputDescriptions vertex_input_descriptions =
            makeVulkanVertexInputDescriptions(vertex_layout,
                                              m_MeshVertexStride);

        const std::array<VkVertexInputBindingDescription, 1> vertex_bindings{
            vertex_input_descriptions.binding,
        };

        const GraphicsVertexInputData vertex_input_data{
            .bindings = vertex_bindings,
            .attributes = vertex_input_descriptions.attributes,
        };

        const std::array<VkPushConstantRange, 1> push_constant_ranges{
            {
                {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0U,
                    .size = static_cast<std::uint32_t>(
                        sizeof(VulkanDrawPushConstants)),
                },
            },
        };

        std::array<const VulkanDescriptorSetLayout *, 1> texture_layouts{};

        std::span<const VulkanDescriptorSetLayout *const>
            descriptor_set_layouts{};

        std::vector<const VulkanDescriptorSetLayout *> layout_references;
        layout_references.reserve(m_DescriptorSetLayouts.size());

        for (const VulkanDescriptorSetLayout &layout : m_DescriptorSetLayouts) {
            layout_references.push_back(&layout);
        }

        if (!layout_references.empty()) {
            descriptor_set_layouts = layout_references;
        } else if (m_TextureDescriptorSetLayout.has_value()) {
            const VulkanDescriptorSetLayout &owned_layout =
                m_TextureDescriptorSetLayout.value();

            texture_layouts[0] = &owned_layout;
            descriptor_set_layouts = texture_layouts;
        }

        const GraphicsPipelineLayoutData graphics_pipeline_layout_data{
            .push_constant_ranges = push_constant_ranges,
            .descriptor_set_layouts = descriptor_set_layouts,
        };

        const GraphicsAttachmentFormats attachment_formats{
            .color = swapchain.surfaceFormat().format,
            .depth = depth_format,
        };

        return m_GraphicsPipelineCache.getOrCreate(
            m_Device.nativeHandle(), attachment_formats, graphics_data,
            vertex_input_data, graphics_pipeline_layout_data,
            m_PhysicalDevice.capabilities.properties.limits
                .maxPushConstantsSize,
            material.pipeline_state);
    }

    auto
    VulkanRenderer::selectGraphicsPipeline(const VulkanSwapchain &swapchain,
                                           VkFormat depth_format)
        -> const VulkanGraphicsPipeline * {
        const VulkanGraphicsPipeline *new_graphics_pipeline{nullptr};

        if (m_ActiveMeshHandle.has_value() &&
            (!m_SwapchainResources.has_value() ||
             swapchain.surfaceFormat().format !=
                 m_SwapchainResources->m_VulkanSwapchain.surfaceFormat()
                     .format ||
             depth_format != m_SwapchainResources->m_DepthFormat)) {

            if (!m_Material.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer requires a material configuration "
                    "when rendering a mesh");
            }

            new_graphics_pipeline = getOrCreateGraphicsPipeline(
                *m_Material, swapchain, depth_format);

            if (new_graphics_pipeline == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer failed to select a graphics pipeline "
                    "for an active mesh");
            }
        }

        if (new_graphics_pipeline == nullptr &&
            m_SwapchainResources.has_value()) {
            new_graphics_pipeline =
                m_SwapchainResources->m_VulkanGraphicsPipeline;
        }

        return new_graphics_pipeline;
    }

    auto VulkanRenderer::recreateSwapchainResources() -> bool {
        if (!waitForDrawableFramebuffer()) {
            return false;
        }

        m_Device.waitIdle();

        for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
            m_GraphicsSubmissionState.recordCompletion(i);
        }

        m_GraphicsResourceRetirementQueue.collectCompleted(
            m_GraphicsSubmissionState);

        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
        if (m_SwapchainResources.has_value()) {
            old_swapchain =
                m_SwapchainResources.value().m_VulkanSwapchain.nativeHandle();
        }

        const Platform::FramebufferSize new_framebuffer_size =
            m_Window->framebufferSize();

        replaceRetiringResource(
            m_SwapchainResources, [&] -> SwapchainResources {
                VulkanSwapchain new_swapchain = VulkanSwapchain(
                    m_PhysicalDevice.handle, m_Device.nativeHandle(),
                    m_Surface.nativeHandle(), m_PhysicalDevice.queue_families,
                    new_framebuffer_size, m_PresentationPreference,
                    m_LogicalDeviceFeatureConfiguration
                            .fifo_latest_ready_feature
                            .presentModeFifoLatestReady == VK_TRUE,
                    old_swapchain);

                const std::optional<VkFormat> depth_format =
                    selectSupportedDepthFormat(m_PhysicalDevice.handle);

                if (!depth_format.has_value()) {
                    throw Core::Error::EngineError(
                        Core::Error::Code::VulkanDepthFormatUnavailable,
                        "No supported Vulkan depth attachment format available",
                        "Select Vulkan Depth Format");
                }

                std::vector<DepthAttachmentResources> new_depth_attachments =
                    createDepthAttachments(new_swapchain.extent(),
                                           depth_format.value());

                const VulkanGraphicsPipeline *new_graphics_pipeline =
                    selectGraphicsPipeline(new_swapchain, depth_format.value());

                std::vector<VulkanSemaphore> new_render_finished_semaphores;
                new_render_finished_semaphores.reserve(
                    new_swapchain.images().size());

                for (std::size_t i{}; i < new_swapchain.images().size(); ++i) {
                    new_render_finished_semaphores.emplace_back(
                        m_Device.nativeHandle());
                }

                return SwapchainResources{
                    .m_VulkanSwapchain = std::move(new_swapchain),
                    .m_DepthFormat = depth_format.value(),
                    .m_VulkanGraphicsPipeline = new_graphics_pipeline,
                    .m_VulkanSemaphores =
                        std::move(new_render_finished_semaphores),
                    .m_DepthAttachments = std::move(new_depth_attachments),
                };
            });

        return true;
    }
} // namespace SNE::Engine::Renderer::Vulkan
