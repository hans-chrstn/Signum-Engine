#include "engine/renderer/material/material_resource_binding_set.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

TEST(MaterialResourceBindingSetTests, DefaultsToEmptyCollection) {
    const Renderer::MaterialResourceBindingSet binding_set{};

    EXPECT_TRUE(binding_set.bindings.empty());
}

TEST(MaterialResourceBindingSetTests, StoresMultipleBindings) {
    const Renderer::MaterialResourceBindingSet binding_set{
        .bindings =
            {
                Renderer::MaterialResourceBinding{
                    .set = 0U,
                    .binding = 0U,
                },
                Renderer::MaterialResourceBinding{
                    .set = 1U,
                    .binding = 2U,
                },
            },
    };

    ASSERT_EQ(binding_set.bindings.size(), 2U);
    EXPECT_EQ(binding_set.bindings[0].set, 0U);
    EXPECT_EQ(binding_set.bindings[1].set, 1U);
    EXPECT_EQ(binding_set.bindings[1].binding, 2U);
}

TEST(MaterialResourceBindingSetTests, EqualCollectionsCompareEqual) {
    const Renderer::MaterialResourceBindingSet first{
        .bindings =
            {
                Renderer::MaterialResourceBinding{.binding = 1U},
                Renderer::MaterialResourceBinding{.binding = 2U},
            },
    };

    const Renderer::MaterialResourceBindingSet second{
        .bindings =
            {
                Renderer::MaterialResourceBinding{.binding = 1U},
                Renderer::MaterialResourceBinding{.binding = 2U},
            },
    };

    EXPECT_EQ(first, second);
}

TEST(MaterialResourceBindingSetTests, BindingOrderAffectsEquality) {
    const Renderer::MaterialResourceBindingSet first{
        .bindings =
            {
                Renderer::MaterialResourceBinding{.binding = 1U},
                Renderer::MaterialResourceBinding{.binding = 2U},
            },
    };

    const Renderer::MaterialResourceBindingSet second{
        .bindings =
            {
                Renderer::MaterialResourceBinding{.binding = 2U},
                Renderer::MaterialResourceBinding{.binding = 1U},
            },
    };

    EXPECT_FALSE(first == second);
}

TEST(MaterialResourceBindingSetTests, CopyOwnsIndependentBindingDescriptions) {
    Renderer::MaterialResourceBindingSet original{
        .bindings =
            {
                Renderer::MaterialResourceBinding{.binding = 1U},
            },
    };

    const Renderer::MaterialResourceBindingSet copy = original;

    original.bindings[0].binding = 3U;

    ASSERT_EQ(copy.bindings.size(), 1U);
    EXPECT_EQ(copy.bindings[0].binding, 1U);
    EXPECT_FALSE(original == copy);
}
