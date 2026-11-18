#include "renderer_feature_requirements.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer {
    namespace {
        [[nodiscard]] auto
        selectCapabilityState(const RendererCapabilities &capabilities,
                              RendererCapability capability)
            -> RendererCapabilityState {

            switch (capability) {
            case RendererCapability::AccelerationStructures:
                return capabilities.acceleration_structures;
            case RendererCapability::RayTracingPipelines:
                return capabilities.ray_tracing_pipelines;
            case RendererCapability::RayQueries:
                return capabilities.ray_queries;
            case RendererCapability::GpuMemoryBudget:
                return capabilities.gpu_memory_budget;
            case RendererCapability::LatestReadyPresentation:
                return capabilities.latest_ready_presentation;
            }

            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "Renderer feature capability evaluation requires a recognized "
                "capability");
        }
    } // namespace

    auto findUnmetRendererFeatureCapability(
        const RendererCapabilities &capabilities,
        std::span<const RendererFeatureCapabilityRequirement> requirements)
        -> std::optional<RendererFeatureCapabilityFailure> {
        for (const RendererFeatureCapabilityRequirement &requirement :
             requirements) {

            const RendererCapabilityState state =
                selectCapabilityState(capabilities, requirement.capability);

            switch (requirement.minimum_state) {
            case RendererCapabilityMinimumState::Supported:
                if (!state.device_supported) {
                    return RendererFeatureCapabilityFailure{
                        .capability = requirement.capability,
                        .reason =
                            RendererFeatureCapabilityFailureReason::Unsupported,
                    };
                }
                continue;
            case RendererCapabilityMinimumState::Enabled:
                if (!state.device_supported) {
                    return RendererFeatureCapabilityFailure{
                        .capability = requirement.capability,
                        .reason =
                            RendererFeatureCapabilityFailureReason::Unsupported,
                    };
                }
                if (!state.device_enabled) {
                    return RendererFeatureCapabilityFailure{
                        .capability = requirement.capability,
                        .reason =
                            RendererFeatureCapabilityFailureReason::NotEnabled,
                    };
                }
                continue;
            }

            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "Renderer feature capability evaluation requires a recognized "
                "minimum state");
        }

        return std::nullopt;
    }
} // namespace SNE::Engine::Renderer
