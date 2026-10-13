#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    class VulkanQueueSubmissionState;

    /**
     * @brief Retains GPU resources until their submissions have completed.
     *
     * Owns deferred resource-release operations associated with
     * queue submission identifiers.
     *
     * Resources remain retained while their associated submissions are
     * incomplete and become eligible for destruction once GPU completion
     * has been confirmed.
     *
     * This class performs CPU-side lifetime management only. It does not
     * own Vulkan queues, fences, command buffers, or synchronization objects.
     *
     * The caller is responsible for associating resources with the correct
     * submission stream, reporting confirmed completion, and ensuring
     * pending resources are handled safely before shutdown.
     *
     * The class is non-copyable to preserve exclusive ownership of
     * deferred resource-release operations.
     */
    class VulkanResourceRetirementQueue {
      private:
        /**
         * @brief Owns a deferred resource-release operation.
         *
         * Associates a resource-release operation with the final queue
         * submission that may reference its resources.
         *
         * The release operation may own move-only RAII resources.
         * Its lifetime must extend until the associated GPU work
         * has completed.
         */
        struct RetiredResource {
            /**
             * @brief Identifier of the last submission using the resource.
             *
             * The resource becomes eligible for destruction once this
             * submission identifier is confirmed complete.
             *
             * The identifier belongs to the submission stream associated
             * with the retirement queue.
             */
            std::uint64_t submission_id{};

            /**
             * @brief Owns the deferred resource-release operation.
             *
             * Supports move-only callables that retain ownership of
             * resources awaiting safe destruction.
             *
             * The stored callable and its captures must not be destroyed
             * before the associated GPU work has completed.
             */
            std::move_only_function<void() noexcept> release;
        };

        /**
         * @brief Resources awaiting confirmed GPU completion.
         *
         * Stores retirement records whose release operations must remain
         * alive until their associated submissions have completed.
         *
         * Completed records are removed during resource collection.
         *
         * Pending records must not be destroyed while their resources
         * remain in use by the GPU.
         */
        std::vector<RetiredResource> m_PendingResources;

      public:
        /**
         * @brief Constructs an empty resource retirement queue.
         *
         * Initializes the queue without pending retirement records.
         *
         * @post The queue contains no pending resources.
         */
        VulkanResourceRetirementQueue() = default;

        /**
         * @brief Destroys the resource retirement queue.
         *
         * Releases the CPU-side storage owned by the queue.
         *
         * @pre All pending retirement records must have been collected
         * before destruction.
         *
         * @note The destructor must assert if pending records remain.
         * The owner must confirm GPU completion and collect retired
         * resources before destroying the queue.
         */
        ~VulkanResourceRetirementQueue() noexcept;

        /**
         * @brief Disables copying of the retirement queue.
         *
         * Deferred resource-release operations have exclusive ownership
         * and must not be duplicated.
         */
        VulkanResourceRetirementQueue(const VulkanResourceRetirementQueue &) =
            delete;

        /**
         * @brief Disables copy assignment of the retirement queue.
         *
         * Prevents duplicating or replacing the queue's ownership
         * of pending resource-release operations.
         */
        auto operator=(const VulkanResourceRetirementQueue &)
            -> VulkanResourceRetirementQueue & = delete;

        /**
         * @brief Retains a resource until its final GPU submission completes.
         *
         * Transfers ownership of a move-only RAII resource into the retirement
         * queue and associates it with the last submission that references it.
         *
         * The resource remains owned by the queue until collection confirms
         * completion of the associated submission.
         *
         * @tparam Resource Type of the resource being retired.
         *
         * @param submission_id Identifier of the final submission using
         * the resource.
         * @param resource Resource whose ownership will be transferred.
         *
         * @pre Resource must be nothrow move-constructible and destructible
         * without throwing exceptions.
         * @pre submission_id must belong to the submission stream used
         * for subsequent completion checks.
         * @pre submission_id must cover all outstanding GPU uses of the
         * resource.
         *
         * @post On success, ownership is transferred to the retirement queue.
         * @post If preparation fails, the caller retains ownership of the
         * unchanged resource.
         *
         * @throws std::bad_alloc if retirement storage allocation fails.
         *
         * @note This function does not wait for GPU completion.
         * @note The implementation must complete all potentially throwing
         * preparation before moving the resource.
         */
        template <typename Resource>
        auto retire(std::uint64_t submission_id, Resource &resource) -> void {
            static_assert(
                std::is_nothrow_move_constructible_v<Resource>,
                "Retired resources must be nothrow move-constructible");

            static_assert(std::is_nothrow_destructible_v<Resource>,
                          "Retired resources must be nothrow destructible");

            auto holder = std::make_unique<std::optional<Resource>>();

            auto *storage = holder.get();

            std::move_only_function<void() noexcept> release =
                [owner = std::move(holder)] noexcept -> void {
                (*owner).reset();
            };

            m_PendingResources.emplace_back(RetiredResource{
                .submission_id = submission_id,
                .release = std::move(release),
            });

            storage->emplace(std::move(resource));
        }

        /**
         * @brief Releases retired resources whose GPU submissions have
         * completed.
         *
         * Examines pending retirement records and releases resources associated
         * with submissions confirmed complete by the supplied submission state.
         *
         * Resources associated with incomplete submissions remain retained.
         *
         * This function does not submit GPU work or wait for GPU completion.
         *
         * @param submission_state Submission completion state used to determine
         * which retired resources are safe to release.
         *
         * @pre The submission state must correspond to the queue whose
         * submissions reference the retired resources.
         * @pre Recorded completion must reflect confirmed GPU execution.
         *
         * @post Resources associated with incomplete submissions remain owned
         * by the retirement queue.
         *
         * @note Resource destruction occurs through the stored release
         * operations. Their exception-safety contract must be established
         * before implementation.
         */
        auto
        collectCompleted(const VulkanQueueSubmissionState &submission_state)
            -> void;
    };
} // namespace SNE::Engine::Renderer::Vulkan
