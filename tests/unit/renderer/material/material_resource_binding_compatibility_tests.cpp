#include "engine/renderer/material/material_resource_binding_compatibility.hpp"
#include "engine/renderer/material/material_resource_binding_set.hpp"
#include <array>
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;

TEST(MaterialResourceBindingCompatibilityTests, AcceptsCompatibleBinding) {
    const Renderer::MaterialResourceBinding provided{
        .set = 0U,
        .binding = 2U,
        .array_element = 0U,
        .type = Shader::ShaderResourceType::CombinedImageSampler,
    };

    const Shader::ShaderDescriptorBinding required{
        .set = 0U,
        .binding = 2U,
        .type = Shader::ShaderResourceType::CombinedImageSampler,
        .count = 1U,
    };

    EXPECT_TRUE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests, RejectsDifferentLocations) {
    const Renderer::MaterialResourceBinding provided{
        .set = 0U,
        .binding = 2U,
    };

    Shader::ShaderDescriptorBinding required{
        .set = 1U,
        .binding = 2U,
        .count = 1U,
    };

    EXPECT_FALSE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));

    required.set = 0U;
    required.binding = 3U;

    EXPECT_FALSE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests, RejectsDifferentResourceTypes) {
    const Renderer::MaterialResourceBinding provided{};

    const Shader::ShaderDescriptorBinding required{
        .type = Shader::ShaderResourceType::UniformBuffer,
        .count = 1U,
    };

    EXPECT_FALSE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests, ChecksDescriptorArrayBounds) {
    Renderer::MaterialResourceBinding provided{};

    const Shader::ShaderDescriptorBinding required{
        .count = 4U,
    };

    provided.array_element = 3U;

    EXPECT_TRUE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));

    provided.array_element = 4U;

    EXPECT_FALSE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests, RejectsZeroDescriptorCount) {
    const Renderer::MaterialResourceBinding provided{};

    const Shader::ShaderDescriptorBinding required{
        .count = 0U,
    };

    EXPECT_FALSE(
        Renderer::isMaterialResourceBindingCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     AcceptsCompleteDescriptorArray) {
    const std::array provided{
        Renderer::MaterialResourceBinding{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
        },
        Renderer::MaterialResourceBinding{
            .set = 0U,
            .binding = 2U,
            .array_element = 1U,
        },
    };

    const std::array required{
        Shader::ShaderDescriptorBinding{
            .set = 0U,
            .binding = 2U,
            .type = Shader::ShaderResourceType::CombinedImageSampler,
            .count = 2U,
        },
    };

    EXPECT_TRUE(
        Renderer::areMaterialResourceBindingsCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     RejectsMissingDescriptorArrayElement) {
    const std::array provided{
        Renderer::MaterialResourceBinding{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
        },
    };

    const std::array required{
        Shader::ShaderDescriptorBinding{.set = 0U, .binding = 2U, .count = 2U},
    };

    EXPECT_FALSE(
        Renderer::areMaterialResourceBindingsCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     RejectsDuplicateLocationsRegardlessOfType) {
    const std::array provided{
        Renderer::MaterialResourceBinding{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
        },
        Renderer::MaterialResourceBinding{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::UniformBuffer,
        },
    };

    EXPECT_FALSE(Renderer::areMaterialResourceBindingsCompatible(provided, {}));
}

TEST(MaterialResourceBindingCompatibilityTests,
     RejectsIncompatibleResourceType) {
    const std::array provided{
        Renderer::MaterialResourceBinding{
            .type = Shader::ShaderResourceType::StorageBuffer,
        },
    };

    const std::array required{
        Shader::ShaderDescriptorBinding{
            .type = Shader::ShaderResourceType::UniformBuffer,
            .count = 1U,
        },
    };

    EXPECT_FALSE(
        Renderer::areMaterialResourceBindingsCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     RejectsZeroDescriptorCountInCollection) {
    const std::array required{
        Shader::ShaderDescriptorBinding{.count = 0U},
    };

    EXPECT_FALSE(Renderer::areMaterialResourceBindingsCompatible({}, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     AcceptsAdditionalMaterialBindings) {
    const std::array provided{
        Renderer::MaterialResourceBinding{.set = 0U, .binding = 0U},
        Renderer::MaterialResourceBinding{.set = 1U, .binding = 0U},
    };

    const std::array required{
        Shader::ShaderDescriptorBinding{.set = 0U, .binding = 0U, .count = 1U},
    };

    EXPECT_TRUE(
        Renderer::areMaterialResourceBindingsCompatible(provided, required));
}

TEST(MaterialResourceBindingCompatibilityTests,
     AcceptsMaterialResourceBindingSet) {
    const Renderer::MaterialResourceBindingSet provided{
        .bindings =
            {
                Renderer::MaterialResourceBinding{
                    .set = 0U,
                    .binding = 1U,
                    .array_element = 0U,
                },
                Renderer::MaterialResourceBinding{
                    .set = 0U,
                    .binding = 1U,
                    .array_element = 1U,
                },
            },
    };

    const std::array required{
        Shader::ShaderDescriptorBinding{
            .set = 0U,
            .binding = 1U,
            .count = 2U,
        },
    };

    EXPECT_TRUE(Renderer::areMaterialResourceBindingsCompatible(
        provided.bindings, required));
}
