#pragma once
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan graphics pipeline and its pipeline layout.
     *
     * Creates and manages the lifetime of a graphics pipeline used for drawing
     * with Vulkan dynamic rendering.
     *
     * The logical device is borrowed and must remain valid for the lifetime of
     * this object.
     *
     * The graphics pipeline and pipeline layout are owned by this object and
     * are destroyed when the object is destroyed.
     */
    class VulkanGraphicsPipeline {
      private:
        /**
         * @brief Logical device used to create and destroy pipeline resources.
         *
         * The handle is borrowed and must remain valid for the lifetime of this
         * object.
         */
        VkDevice m_Device{VK_NULL_HANDLE};

        /**
         * @brief Vulkan pipeline layout owned by this object.
         */
        VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};

        /**
         * @brief Vulkan graphics pipeline handle owned by this object.
         */
        VkPipeline m_Pipeline{VK_NULL_HANDLE};

      public:
        /**
         * @brief Creates a Vulkan graphics pipeline for the supplied color
         * attachment format.
         *
         * @param device Logical device used to create and destroy pipeline
         * resources.
         * @param color_attachment_format Format of the color attachment used
         * with the graphics pipeline.
         *
         * @throws Core::Error::EngineError if creation of the pipeline layout
         * or graphics pipeline fails.
         */
        VulkanGraphicsPipeline(VkDevice device,
                               VkFormat color_attachment_format);

        VulkanGraphicsPipeline(const VulkanGraphicsPipeline &) = delete;

        auto operator=(const VulkanGraphicsPipeline &)
            -> VulkanGraphicsPipeline & = delete;

        /**
         * @brief Destroys the owned graphics pipeline and pipeline layout.
         */
        ~VulkanGraphicsPipeline() noexcept;

        /**
         * @brief Returns the underlying Vulkan graphics pipeline handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the graphics pipeline.
         *
         * @return Vulkan graphics pipeline handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkPipeline;
    };
} // namespace SNE::Engine::Renderer::Vulkan
