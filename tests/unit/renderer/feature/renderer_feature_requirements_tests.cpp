#include "engine/renderer/feature/renderer_feature_requirements.hpp"
#include "engine/renderer/renderer_capabilities.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    auto expectCapabilityFailure(
        const std::optional<Renderer::RendererFeatureCapabilityFailure> &actual,
        Renderer::RendererCapability expected_capability,
        Renderer::RendererFeatureCapabilityFailureReason expected_reason)
        -> void {
        if (!actual.has_value()) {
            FAIL() << "Expected an unmet renderer capability requirement";
        }

        EXPECT_EQ(actual->capability, expected_capability);
        EXPECT_EQ(actual->reason, expected_reason);
    }

    struct CapabilityTestCase {
        Renderer::RendererCapability capability;
        Renderer::RendererCapabilities capabilities;
    };
} // namespace

TEST(RendererFeatureRequirementsTests, EmptyRequirementsAreSatisfied) {
    const Renderer::RendererCapabilities capabilities{};

    const auto failure =
        Renderer::findUnmetRendererFeatureCapability(capabilities, {});

    EXPECT_FALSE(failure.has_value());
}

TEST(RendererFeatureRequirementsTests,
     AcceptsSupportedCapabilityWithoutEnablement) {
    Renderer::RendererCapabilities capabilities{};
    capabilities.ray_queries = {
        .device_supported = true,
        .device_enabled = false,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state =
                Renderer::RendererCapabilityMinimumState::Supported,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    EXPECT_FALSE(failure.has_value());
}

TEST(RendererFeatureRequirementsTests, AcceptsEnabledCapability) {
    Renderer::RendererCapabilities capabilities{};
    capabilities.ray_queries = {
        .device_supported = true,
        .device_enabled = true,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    EXPECT_FALSE(failure.has_value());
}

TEST(RendererFeatureRequirementsTests, ReportsUnsupportedWhenSupportRequired) {
    const Renderer::RendererCapabilities capabilities{};

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state =
                Renderer::RendererCapabilityMinimumState::Supported,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::Unsupported);
}

TEST(RendererFeatureRequirementsTests,
     ReportsUnsupportedWhenEnablementRequired) {
    const Renderer::RendererCapabilities capabilities{};

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::Unsupported);
}

TEST(RendererFeatureRequirementsTests,
     ReportsNotEnabledWhenSupportedButDisabled) {
    Renderer::RendererCapabilities capabilities{};
    capabilities.ray_queries = {
        .device_supported = true,
        .device_enabled = false,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::NotEnabled);
}

TEST(RendererFeatureRequirementsTests,
     RejectsEnabledStateWithoutDeviceSupport) {
    Renderer::RendererCapabilities capabilities{};
    capabilities.ray_queries = {
        .device_supported = false,
        .device_enabled = true,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::Unsupported);
}

TEST(RendererFeatureRequirementsTests,
     MapsEveryCapabilityToItsCorrespondingDeviceState) {
    std::array cases{
        CapabilityTestCase{
            .capability = Renderer::RendererCapability::AccelerationStructures,
            .capabilities = {},
        },
        CapabilityTestCase{
            .capability = Renderer::RendererCapability::RayTracingPipelines,
            .capabilities = {},
        },
        CapabilityTestCase{
            .capability = Renderer::RendererCapability::RayQueries,
            .capabilities = {},
        },
        CapabilityTestCase{
            .capability = Renderer::RendererCapability::GpuMemoryBudget,
            .capabilities = {},
        },
        CapabilityTestCase{
            .capability = Renderer::RendererCapability::LatestReadyPresentation,
            .capabilities = {},
        },
    };

    cases[0].capabilities.acceleration_structures = {
        .device_supported = true,
        .device_enabled = true,
    };

    cases[1].capabilities.ray_tracing_pipelines = {
        .device_supported = true,
        .device_enabled = true,
    };

    cases[2].capabilities.ray_queries = {
        .device_supported = true,
        .device_enabled = true,
    };

    cases[3].capabilities.gpu_memory_budget = {
        .device_supported = true,
        .device_enabled = true,
    };

    cases[4].capabilities.latest_ready_presentation = {
        .device_supported = true,
        .device_enabled = true,
    };

    for (const CapabilityTestCase &test_case : cases) {
        SCOPED_TRACE(static_cast<int>(test_case.capability));

        const std::array requirements{
            Renderer::RendererFeatureCapabilityRequirement{
                .capability = test_case.capability,
                .minimum_state =
                    Renderer::RendererCapabilityMinimumState::Enabled,
            },
        };

        const auto failure = Renderer::findUnmetRendererFeatureCapability(
            test_case.capabilities, requirements);

        EXPECT_FALSE(failure.has_value());
    }
}

TEST(RendererFeatureRequirementsTests,
     ReturnsFirstUnmetRequirementInInputOrder) {
    Renderer::RendererCapabilities capabilities{};

    capabilities.ray_queries = {
        .device_supported = true,
        .device_enabled = false,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::GpuMemoryBudget,
            .minimum_state =
                Renderer::RendererCapabilityMinimumState::Supported,
        },
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto first_failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        first_failure, Renderer::RendererCapability::GpuMemoryBudget,
        Renderer::RendererFeatureCapabilityFailureReason::Unsupported);

    const std::array reversed_requirements{
        requirements[1],
        requirements[0],
    };

    const auto reversed_failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, reversed_requirements);

    expectCapabilityFailure(
        reversed_failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::NotEnabled);
}

TEST(RendererFeatureRequirementsTests, ContinuesPastSatisfiedRequirements) {
    Renderer::RendererCapabilities capabilities{};

    capabilities.gpu_memory_budget = {
        .device_supported = true,
        .device_enabled = false,
    };

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::GpuMemoryBudget,
            .minimum_state =
                Renderer::RendererCapabilityMinimumState::Supported,
        },
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    const auto failure = Renderer::findUnmetRendererFeatureCapability(
        capabilities, requirements);

    expectCapabilityFailure(
        failure, Renderer::RendererCapability::RayQueries,
        Renderer::RendererFeatureCapabilityFailureReason::Unsupported);
}

TEST(RendererFeatureRequirementsTests, RejectsUnknownCapabilityIdentifier) {
    const Renderer::RendererCapabilities capabilities{};

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability =
                std::bit_cast<Renderer::RendererCapability>(std::uint8_t{255U}),
            .minimum_state = Renderer::RendererCapabilityMinimumState::Enabled,
        },
    };

    ASSERT_DEATH(
        {
            static_cast<void>(Renderer::findUnmetRendererFeatureCapability(
                capabilities, requirements));
        },
        "requires a recognized capability");
}

TEST(RendererFeatureRequirementsTests,
     RejectsUnknownMinimumStateEvenWhenUnsupported) {
    const Renderer::RendererCapabilities capabilities{};

    const std::array requirements{
        Renderer::RendererFeatureCapabilityRequirement{
            .capability = Renderer::RendererCapability::RayQueries,
            .minimum_state =
                std::bit_cast<Renderer::RendererCapabilityMinimumState>(
                    std::uint8_t{255U}),
        },
    };

    ASSERT_DEATH(
        {
            static_cast<void>(Renderer::findUnmetRendererFeatureCapability(
                capabilities, requirements));
        },
        "requires a recognized minimum state");
}
