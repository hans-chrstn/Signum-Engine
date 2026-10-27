#pragma once

#include "engine/renderer/vulkan/memory/vulkan_sampler.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_graphics_submission_progress.hpp"
#include "engine/renderer/vulkan/vulkan_scene_viewport_image_info.hpp"
#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace SNE::Editor {
    /**
     * @brief Owns the Dear ImGui Vulkan rendering backend.
     *
     * Initializes Dear ImGui's Vulkan backend using borrowed Vulkan handles
     * and swapchain configuration supplied by the engine renderer.
     *
     * The backend manages its own ImGui rendering resources but does not
     * own the Vulkan instance, physical device, logical device, graphics
     * queue, or swapchain.
     *
     * An initialized Dear ImGui context must exist before construction
     * and remain alive until after this object is destroyed.
     *
     * The engine renderer and its Vulkan device must also outlive this
     * object.
     *
     * This class is non-copyable and non-movable because it manages the
     * lifetime of a backend associated with the active ImGui context.
     */
    class EditorImGuiVulkanBackend {
      private:
        /**
         * @brief Borrowed logical-device handle used during backend shutdown.
         *
         * Remains owned by the engine renderer and must outlive this object.
         * VK_NULL_HANDLE indicates that initialization has not completed.
         */
        VkDevice m_Device{VK_NULL_HANDLE};

        /**
         * @brief Owns the sampler used to display offscreen scene images.
         *
         * Provides the sampling configuration for scene viewport textures
         * registered with the Dear ImGui Vulkan backend.
         *
         * The sampler is independent of the renderer-owned scene images
         * and their image views.
         *
         * Initialized when the editor's scene viewport sampling resources
         * are created.
         *
         * Must remain valid while GPU submissions reference descriptors
         * that use this sampler.
         *
         * @note Does not own scene images or their image views.
         */
        std::optional<Vulkan::VulkanSampler> m_SceneViewportSampler;

        /**
         * @brief Tracks a scene viewport texture registered with Dear ImGui.
         *
         * Stores the descriptor set, borrowed image view, and resource
         * generation associated with an offscreen scene texture.
         *
         * The descriptor set is allocated and managed through the Dear ImGui
         * Vulkan backend. The renderer retains ownership of the image view.
         *
         * Registration information allows the editor to reuse existing
         * texture descriptors when the underlying image remains unchanged
         * and detect when resource replacement requires registration updates.
         *
         * Registered descriptors must not be released while outstanding GPU
         * submissions reference them.
         *
         * The referenced image view and sampler must remain valid while
         * the descriptor is used by the GPU.
         *
         * @note Does not own the underlying Vulkan image, image view,
         * sampler, or descriptor pool.
         * @note Resource generation tracking does not guarantee GPU
         * completion or extend the lifetime of borrowed resources.
         */
        struct SceneViewportTextureRegistration {
            /**
             * @brief Descriptor set identifying the registered ImGui texture.
             *
             * Created through the Dear ImGui Vulkan backend and used when
             * displaying the offscreen scene image.
             *
             * VK_NULL_HANDLE indicates that no texture is registered.
             *
             * Must be unregistered through the appropriate ImGui backend
             * operation when no longer needed and safe to release.
             */
            VkDescriptorSet m_DescriptorSet{VK_NULL_HANDLE};

            /**
             * @brief Borrowed image view associated with the texture
             * descriptor.
             *
             * References the offscreen color image view owned by
             * VulkanRenderer.
             *
             * Used to identify the image associated with the current
             * texture registration.
             *
             * VK_NULL_HANDLE indicates that no image view is associated
             * with the registration.
             *
             * @note Does not transfer ownership of the image view.
             */
            VkImageView m_ImageView{VK_NULL_HANDLE};

            /**
             * @brief Offscreen resource generation associated with the
             * registration.
             *
             * Identifies the renderer resource allocation for which the
             * texture descriptor was created.
             *
             * Allows the editor to detect when offscreen resources have
             * been replaced and the texture registration may need updating.
             *
             * Zero indicates that no resource generation has been assigned.
             *
             * @note A generation mismatch does not mean the previous
             * descriptor can immediately be released.
             */
            std::uint64_t m_Generation{};

            /**
             * @brief Identifies the latest graphics submission using this
             * descriptor.
             *
             * Records the submission identifier of the most recent successfully
             * submitted GPU work that references this texture registration.
             *
             * Used to determine when an outdated descriptor can be safely
             * released after confirmed GPU completion.
             *
             * Zero indicates that no submitted GPU usage has been recorded.
             *
             * @note The identifier must correspond to an actual successful
             * graphics submission, not a predicted submission.
             * @note This value does not extend the lifetime of the
             * renderer-owned image view.
             */
            std::uint64_t m_LastUsedSubmissionId{};
        };

        /**
         * @brief Tracks ImGui texture registrations for scene viewport frame
         * slots.
         *
         * Stores descriptor registrations associated with the renderer's
         * per-frame offscreen scene color images.
         *
         * Each entry is indexed by the corresponding renderer frame slot.
         * An entry with a null descriptor set represents an unregistered
         * texture.
         *
         * Registrations are created on demand when the editor receives
         * valid scene viewport image information.
         *
         * Entries may reference different resource generations while
         * viewport replacement and descriptor retirement are coordinated.
         *
         * The collection does not own renderer images or image views.
         *
         * Registered descriptors must be released only after outstanding
         * GPU operations that reference them have completed.
         *
         * @note The number of renderer frame slots is independent of
         * the swapchain image count.
         * @note Destroying the collection does not automatically unregister
         * its ImGui texture descriptors.
         */
        std::vector<SceneViewportTextureRegistration>
            m_SceneViewportTextureRegistrations;

        /**
         * @brief Retains replaced scene viewport texture registrations.
         *
         * Stores outdated ImGui texture registrations that may still be
         * referenced by outstanding graphics submissions.
         *
         * Entries remain tracked until their associated GPU usage has
         * completed and their descriptor sets can be safely released.
         *
         * Each registration preserves its descriptor set, borrowed image
         * view, resource generation, and submission tracking information.
         *
         * The editor owns descriptor registration management but does
         * not own the underlying renderer image views.
         *
         * @note Resource generation changes do not establish GPU completion.
         * @note Removing entries from this collection does not automatically
         * unregister their ImGui descriptors.
         */
        std::vector<SceneViewportTextureRegistration>
            m_RetiredSceneViewportTextureRegistrations;

      public:
        /**
         * @brief Initializes the Dear ImGui Vulkan rendering backend.
         *
         * Configures ImGui to render using the engine's existing Vulkan device,
         * graphics queue, and swapchain attachment format.
         *
         * @param rendering_info Borrowed Vulkan handles and current rendering
         * configuration supplied by the engine renderer.
         *
         * @pre A valid Dear ImGui context must be active.
         * @pre All required Vulkan handles must be valid.
         * @pre The swapchain must have a valid color attachment format and
         * at least two images.
         *
         * @throws SNE::Engine::Core::Error::EngineError if backend
         * initialization fails.
         */
        explicit EditorImGuiVulkanBackend(
            const Vulkan::VulkanExternalRenderingInfo &rendering_info);

        /**
         * @brief Shuts down the Dear ImGui Vulkan rendering backend.
         *
         * Ensures outstanding GPU operations have completed before releasing
         * backend-owned Vulkan resources.
         *
         * Does not destroy the borrowed logical device or other engine-owned
         * Vulkan resources.
         */
        ~EditorImGuiVulkanBackend();

        EditorImGuiVulkanBackend(const EditorImGuiVulkanBackend &) = delete;
        auto operator=(const EditorImGuiVulkanBackend &)
            -> EditorImGuiVulkanBackend & = delete;

        EditorImGuiVulkanBackend(EditorImGuiVulkanBackend &&) = delete;
        auto operator=(EditorImGuiVulkanBackend &&)
            -> EditorImGuiVulkanBackend & = delete;

        /**
         * @brief Finds or registers an ImGui texture for an offscreen scene
         * image.
         *
         * Reuses an existing texture registration when its frame slot,
         * image view, and resource generation match the supplied information.
         *
         * A new registration is created when no matching descriptor exists.
         *
         * The renderer retains ownership of the underlying image and image
         * view. The editor manages the descriptor registration.
         *
         * @param image_info Borrowed information describing the scene color
         * image.
         *
         * @return Descriptor set identifying the registered ImGui texture.
         *
         * @pre The ImGui Vulkan backend and scene viewport sampler must
         * be initialized.
         * @pre The supplied image view must be valid.
         * @pre Descriptor and image lifetimes must be coordinated with
         * outstanding graphics submissions.
         *
         * @note The upcoming frame submission has not necessarily been
         * recorded when this method is called.
         * @note Does not transfer ownership of the supplied image view.
         * @note Replaced registrations must not be released until their
         * outstanding GPU uses have completed.
         */
        [[nodiscard]] auto sceneViewportTexture(
            const Vulkan::VulkanSceneViewportImageInfo &image_info)
            -> VkDescriptorSet;

        /**
         * @brief Records successful GPU submission of a scene viewport texture.
         *
         * Associates an existing ImGui texture registration with the
         * graphics submission that references its descriptor set.
         *
         * Searches tracked active and retired registrations to identify
         * the submitted texture.
         *
         * Updates the registration's latest submitted usage identifier
         * without allocating or releasing Vulkan resources.
         *
         * @param descriptor_set Descriptor set used by the submitted frame.
         * @param submission_id Identifier of the successful graphics
         * submission.
         *
         * @pre descriptor_set must identify a tracked texture registration.
         * @pre submission_id must identify an actual successful graphics
         * submission from the renderer's graphics queue.
         * @pre Calls must be serialized with texture registration and
         * retirement operations.
         *
         * @post The matching registration records its latest GPU usage.
         *
         * @note This operation does not establish GPU completion.
         * @note Descriptor retirement requires separately confirmed
         * completion of its final usage.
         */
        auto
        recordSceneViewportTextureSubmission(VkDescriptorSet descriptor_set,
                                             std::uint64_t submission_id)
            -> void;

        /**
         * @brief Releases retired scene viewport texture registrations.
         *
         * Examines retired ImGui texture descriptors and unregisters those
         * whose final GPU usage has been confirmed complete.
         *
         * Uses the renderer's graphics submission progress to determine
         * which registrations are eligible for cleanup.
         *
         * Registrations associated with outstanding GPU work remain
         * tracked for later collection.
         *
         * @param submission_progress Snapshot of the renderer's graphics
         * submission and completion identifiers.
         *
         * @pre The Dear ImGui Vulkan backend must be initialized.
         * @pre Calls must be serialized with texture registration,
         * GPU submission tracking, and backend shutdown.
         * @pre No unsubmitted ImGui draw data may reference a descriptor
         * selected for removal.
         *
         * @post Eligible retired descriptors are unregistered.
         * @post Registrations awaiting GPU completion remain tracked.
         *
         * @note This operation does not wait for GPU completion.
         * @note The renderer retains ownership of the underlying images.
         * @note The completion snapshot must correspond to the same
         * graphics submission stream used for descriptor tracking.
         */
        auto collectRetiredSceneViewportTextures(
            const Vulkan::VulkanGraphicsSubmissionProgress &submission_progress)
            -> void;
    };
} // namespace SNE::Editor
