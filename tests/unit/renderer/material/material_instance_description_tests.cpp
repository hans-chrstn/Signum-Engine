#include "engine/renderer/material/material_instance_description.hpp"
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

TEST(MaterialInstanceDescriptionTests, DefaultsToEmptyResourceBindings) {
    const Renderer::MaterialInstanceDescription description{};

    EXPECT_TRUE(description.resource_bindings.bindings.empty());
}

TEST(MaterialInstanceDescriptionTests, StoresTextureAndBufferBindings) {
    const Renderer::GpuResourceHandle texture_handle{
        .index = 1U,
        .generation = 2U,
        .registry_id = 10U,
    };

    const Renderer::GpuResourceHandle buffer_handle{
        .index = 3U,
        .generation = 4U,
        .registry_id = 20U,
    };

    const Renderer::MaterialInstanceDescription description{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{
                            .set = 0U,
                            .binding = 0U,
                            .resource_handle = texture_handle,
                        },
                        Renderer::MaterialResourceBinding{
                            .set = 0U,
                            .binding = 1U,
                            .type = Renderer::Shader::ShaderResourceType::
                                UniformBuffer,
                            .resource_handle = buffer_handle,
                        },
                    },
            },
    };

    ASSERT_EQ(description.resource_bindings.bindings.size(), 2U);

    EXPECT_EQ(description.resource_bindings.bindings[0].resource_handle,
              texture_handle);

    EXPECT_EQ(description.resource_bindings.bindings[1].resource_handle,
              buffer_handle);

    EXPECT_EQ(description.resource_bindings.bindings[1].type,
              Renderer::Shader::ShaderResourceType::UniformBuffer);
}

TEST(MaterialInstanceDescriptionTests, EqualDescriptionsCompareEqual) {
    const Renderer::MaterialInstanceDescription first{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{
                            .binding = 1U,
                        },
                    },
            },
    };

    const Renderer::MaterialInstanceDescription second{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{
                            .binding = 1U,
                        },
                    },
            },
    };

    EXPECT_EQ(first, second);
}

TEST(MaterialInstanceDescriptionTests, DifferentResourcesCompareUnequal) {
    const Renderer::MaterialInstanceDescription original{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{
                            .binding = 1U,
                            .resource_handle =
                                {
                                    .index = 1U,
                                    .generation = 1U,
                                    .registry_id = 10U,
                                },
                        },
                    },
            },
    };

    Renderer::MaterialInstanceDescription modified = original;

    modified.resource_bindings.bindings[0].resource_handle.index = 2U;

    EXPECT_FALSE(original == modified);
}

TEST(MaterialInstanceDescriptionTests, BindingOrderAffectsEquality) {
    const Renderer::MaterialInstanceDescription first{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{.binding = 1U},
                        Renderer::MaterialResourceBinding{.binding = 2U},
                    },
            },
    };

    const Renderer::MaterialInstanceDescription second{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{.binding = 2U},
                        Renderer::MaterialResourceBinding{.binding = 1U},
                    },
            },
    };

    EXPECT_FALSE(first == second);
}

TEST(MaterialInstanceDescriptionTests, CopiesOwnIndependentBindingStorage) {
    Renderer::MaterialInstanceDescription original{
        .resource_bindings =
            {
                .bindings =
                    {
                        Renderer::MaterialResourceBinding{.binding = 1U},
                    },
            },
    };

    const Renderer::MaterialInstanceDescription copy = original;

    original.resource_bindings.bindings[0].binding = 3U;

    ASSERT_EQ(copy.resource_bindings.bindings.size(), 1U);
    EXPECT_EQ(copy.resource_bindings.bindings[0].binding, 1U);
    EXPECT_FALSE(original == copy);
}
