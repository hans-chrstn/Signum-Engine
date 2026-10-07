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

        /**
         * @brief Releases the currently owned pipeline resources.
         *
         * Destroys the owned graphics pipeline before destroying the owned
         * pipeline layout, then resets all stored Vulkan handles to an empty
         * non-owning state.
         *
         * Safe to call when no pipeline resources are owned.
         */
        auto destroy() noexcept -> void;

      public:
        /**
         * @brief Creates a Vulkan graphics pipeline for the supplied color
         * attachment format.
         *
         * Creates and owns the pipeline layout and graphics pipeline used for
         * Vulkan dynamic rendering with the supplied color-attachment format.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this object.
         *
         * @param device Logical device used to create and destroy pipeline
         * resources.
         * @param color_attachment_format Format of the color attachment used
         * with the graphics pipeline.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre color_attachment_format must not be VK_FORMAT_UNDEFINED.
         *
         * @post Successful pipeline-layout creation produces a non-null Vulkan
         * pipeline-layout handle.
         * @post Successful graphics-pipeline creation produces a non-null
         * Vulkan graphics-pipeline handle.
         *
         * @throws Core::Error::EngineError if shader loading, shader-module
         * creation, pipeline-layout creation, or graphics-pipeline creation
         * fails.
         */
        VulkanGraphicsPipeline(VkDevice device,
                               VkFormat color_attachment_format);

        VulkanGraphicsPipeline(const VulkanGraphicsPipeline &) = delete;

        auto operator=(const VulkanGraphicsPipeline &)
            -> VulkanGraphicsPipeline & = delete;

        /**
         * @brief Transfers ownership of graphics-pipeline resources.
         *
         * Transfers the logical-device handle, pipeline layout, and graphics
         * pipeline from the source object.
         *
         * The source object is left in a valid moved-from state containing no
         * owned Vulkan pipeline resources.
         *
         * @param other Pipeline object whose resources are transferred.
         */
        VulkanGraphicsPipeline(VulkanGraphicsPipeline &&other) noexcept;

        /**
         * @brief Replaces the currently owned resources by moving from another
         * pipeline object.
         *
         * Releases any pipeline resources currently owned by this object before
         * transferring the source object's logical-device handle, pipeline
         * layout, and graphics pipeline.
         *
         * Self-move assignment has no effect.
         *
         * The source object is left in a valid moved-from state containing no
         * owned Vulkan pipeline resources.
         *
         * @param other Pipeline object whose resources are transferred.
         *
         * @return Reference to this pipeline object.
         */
        auto operator=(VulkanGraphicsPipeline &&other) noexcept
            -> VulkanGraphicsPipeline &;

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
