#include "engine/renderer/material/material_resource_binding.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;

TEST(MaterialResourceBindingTests, DefaultBindingsCompareEqual) {
    const Renderer::MaterialResourceBinding first{};
    const Renderer::MaterialResourceBinding second{};

    EXPECT_TRUE(first == second);
    EXPECT_EQ(first.set, 0U);
    EXPECT_EQ(first.binding, 0U);
    EXPECT_EQ(first.array_element, 0U);
    EXPECT_EQ(first.type, Shader::ShaderResourceType::CombinedImageSampler);
}

TEST(MaterialResourceBindingTests, DifferentLocationsCompareUnequal) {
    const Renderer::MaterialResourceBinding original{};
    Renderer::MaterialResourceBinding modified{};

    modified.set = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.binding = 2U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.array_element = 3U;
    EXPECT_FALSE(original == modified);
}

TEST(MaterialResourceBindingTests, DifferentResourceTypesCompareUnequal) {
    const Renderer::MaterialResourceBinding original{};
    Renderer::MaterialResourceBinding modified{};

    modified.type = Shader::ShaderResourceType::UniformBuffer;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialResourceBindingTests, DifferentResourceIdentitiesCompareUnequal) {
    const Renderer::MaterialResourceBinding original{};
    Renderer::MaterialResourceBinding modified{};

    modified.resource_handle.index = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.resource_handle.generation = 1U;
    EXPECT_FALSE(original == modified);

    modified = original;
    modified.resource_handle.registry_id = 1U;
    EXPECT_FALSE(original == modified);
}
