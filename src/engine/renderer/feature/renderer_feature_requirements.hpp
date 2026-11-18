#pragma once

#include "engine/renderer/renderer_capabilities.hpp"
#include <cstdint>
#include <optional>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies an optional renderer device capability.
     *
     * Provides backend-independent identifiers for capabilities
     * discovered and negotiated during renderer initialization.
     *
     * These identifiers describe device functionality rather than
     * complete rendering effects.
     */
    enum class RendererCapability : std::uint8_t {
        /** Acceleration-structure functionality. */
        AccelerationStructures,

        /** Hardware ray-tracing pipeline functionality. */
        RayTracingPipelines,

        /** Hardware ray-query functionality. */
        RayQueries,

        /** GPU memory-budget reporting functionality. */
        GpuMemoryBudget,

        /** FIFO latest-ready presentation functionality. */
        LatestReadyPresentation,
    };

    /**
     * @brief Specifies the minimum device state required by a render feature.
     *
     * Supported requires hardware and API support without requiring
     * the functionality to be enabled on the active device.
     *
     * Enabled requires functionality to be both supported and enabled
     * during logical-device initialization.
     */
    enum class RendererCapabilityMinimumState : std::uint8_t {
        /** The selected device must support the capability. */
        Supported,

        /** The capability must be supported and enabled. */
        Enabled,
    };

    /**
     * @brief Describes one device capability required by a rendering feature.
     *
     * Associates a capability identifier with the minimum device state
     * needed to satisfy the feature's requirements.
     *
     * Does not own resources or perform feature initialization.
     */
    struct RendererFeatureCapabilityRequirement {
        /** Capability required by the rendering feature. */
        RendererCapability capability{};

        /** Minimum device state required for the capability. */
        RendererCapabilityMinimumState minimum_state{};
    };

    /**
     * @brief Identifies why a renderer capability requirement is unmet.
     *
     * Distinguishes functionality unavailable on the selected device
     * from supported functionality not enabled during initialization.
     */
    enum class RendererFeatureCapabilityFailureReason : std::uint8_t {
        /** The selected device does not support the capability. */
        Unsupported,

        /** The capability is supported but not enabled. */
        NotEnabled,
    };

    /**
     * @brief Describes an unmet rendering-feature capability requirement.
     *
     * Identifies the required capability and why the active renderer
     * cannot currently satisfy it.
     *
     * This description is backend-independent and does not own
     * diagnostic or GPU resources.
     */
    struct RendererFeatureCapabilityFailure {
        /** Capability whose minimum requirement was not satisfied. */
        RendererCapability capability{};

        /** Reason the required device state was unavailable. */
        RendererFeatureCapabilityFailureReason reason{};
    };

    /**
     * @brief Finds the first unmet renderer feature capability requirement.
     *
     * Evaluates requirements against previously discovered and enabled
     * device capabilities without interacting with the graphics backend.
     *
     * Requirements are evaluated in their supplied order.
     *
     * @param capabilities Snapshot of supported and enabled device
     * functionality.
     * @param requirements Borrowed capability requirements to evaluate.
     *
     * @return The first unmet requirement and its failure reason,
     * or std::nullopt when every requirement is satisfied.
     *
     * @pre Requirement entries must contain recognized capability
     * identifiers and minimum-state values.
     *
     * @post Input descriptions and renderer state remain unchanged.
     *
     * @note Unsupported optional functionality is returned as a
     * normal result rather than treated as an assertion failure.
     *
     * @note The function does not allocate GPU resources, perform
     * Vulkan operations, or retain borrowed requirement storage.
     */
    [[nodiscard]] auto findUnmetRendererFeatureCapability(
        const RendererCapabilities &capabilities,
        std::span<const RendererFeatureCapabilityRequirement> requirements)
        -> std::optional<RendererFeatureCapabilityFailure>;

} // namespace SNE::Engine::Renderer
