#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/lighting/directional_light_uniform_data.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_buffer_update_frequency.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_surface.hpp"
#include "engine/renderer/vulkan/vulkan_instance.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Core = SNE::Engine::Core;
namespace Math = Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Shader = Renderer::Shader;
namespace Vulkan = Renderer::Vulkan;

namespace {
    struct TestVertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
        std::array<float, 3> normal;
        std::array<float, 2> uv;
        std::array<float, 4> tangent;
    };

    static_assert(std::is_standard_layout_v<TestVertex>);

    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr std::array kVertices{
        TestVertex{
            .position = {-0.5F, -0.5F, 0.0F},
            .color = {1.0F, 0.0F, 0.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {0.0F, 0.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
        TestVertex{
            .position = {0.5F, -0.5F, 0.0F},
            .color = {0.0F, 1.0F, 0.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {1.0F, 0.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
        TestVertex{
            .position = {0.0F, 0.5F, 0.0F},
            .color = {0.0F, 0.0F, 1.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {0.5F, 1.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
    };

    constexpr std::array<std::uint16_t, 3> kIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kTriangleVertexAttributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset =
                static_cast<std::uint32_t>(offsetof(TestVertex, position)),
        },
        Renderer::VertexAttributeDescription{
            .location = 1U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, color)),
        },
    };

    constexpr std::array kTexturedVertexAttributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset =
                static_cast<std::uint32_t>(offsetof(TestVertex, position)),
        },
        Renderer::VertexAttributeDescription{
            .location = 1U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, normal)),
        },
        Renderer::VertexAttributeDescription{
            .location = 2U,
            .format = Renderer::VertexFormat::Float32x2,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, uv)),
        },
        Renderer::VertexAttributeDescription{
            .location = 3U,
            .format = Renderer::VertexFormat::Float32x4,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, tangent)),
        },
    };

    constexpr std::array kMaterialTexturedVertexAttributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset =
                static_cast<std::uint32_t>(offsetof(TestVertex, position)),
        },
        Renderer::VertexAttributeDescription{
            .location = 1U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, normal)),
        },
        Renderer::VertexAttributeDescription{
            .location = 2U,
            .format = Renderer::VertexFormat::Float32x2,
            .offset = static_cast<std::uint32_t>(offsetof(TestVertex, uv)),
        },
    };

    constexpr std::array<std::uint8_t, 16> kInitialPixels{
        255U, 0U, 0U,   255U, 0U,   255U, 0U,   255U,
        0U,   0U, 255U, 255U, 255U, 255U, 255U, 255U,
    };

    constexpr std::array<std::uint8_t, 16> kReplacementPixels{
        255U, 255U, 0U,   255U, 0U, 255U, 255U, 255U,
        255U, 0U,   255U, 255U, 0U, 0U,   0U,   255U,
    };

    constexpr std::array<std::uint8_t, 4> kThirdPixels{
        128U,
        64U,
        32U,
        255U,
    };

    [[nodiscard]] auto makeInitialTexture() -> Renderer::TextureUploadData {
        return {
            .width = 2U,
            .height = 2U,
            .pixels = std::as_bytes(std::span{kInitialPixels}),
        };
    }

    [[nodiscard]] auto makeReplacementTexture() -> Renderer::TextureUploadData {
        return {
            .width = 4U,
            .height = 1U,
            .pixels = std::as_bytes(std::span{kReplacementPixels}),
        };
    }

    [[nodiscard]] auto makeThirdTexture() -> Renderer::TextureUploadData {
        return {
            .width = 1U,
            .height = 1U,
            .pixels = std::as_bytes(std::span{kThirdPixels}),
        };
    }

    [[nodiscard]] auto makeInitialMesh(
        std::span<const Math::Matrix4f> model_matrices,
        std::span<const Renderer::VertexAttributeDescription> attributes)
        -> Renderer::InitialMeshData {
        return {
            .mesh =
                {
                    .vertex_bytes = std::as_bytes(std::span{kVertices}),
                    .vertex_stride =
                        static_cast<std::uint32_t>(sizeof(TestVertex)),
                    .indices = kIndices,
                },
            .layout =
                {
                    .attributes = attributes,
                },
            .model_matrices = model_matrices,
        };
    }

    [[nodiscard]] auto
    makeInitialMesh(std::span<const Math::Matrix4f> model_matrices)
        -> Renderer::InitialMeshData {
        return makeInitialMesh(model_matrices, kTriangleVertexAttributes);
    }

    [[nodiscard]] auto makeGraphicsShaders() -> Shader::GraphicsShaderData {
        return {
            .vertex =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        "triangle.vert.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        "triangle.frag.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };
    }

    [[nodiscard]] auto makeTexturedGraphicsShaders()
        -> Shader::GraphicsShaderData {
        return {
            .vertex =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        "textured.vert.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        "textured.frag.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };
    }

    [[nodiscard]] auto makeTwoTextureGraphicsShaders()
        -> Shader::GraphicsShaderData {

        Shader::GraphicsShaderData shaders = makeTexturedGraphicsShaders();

        shaders.fragment.artifact =
            Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                              "material_two_textures.frag.spv");

        return shaders;
    }

    constexpr std::array<std::uint8_t, 20U> kMipmappedPixels{
        255U, 0U,   0U,   255U, 0U,   255U, 0U,   255U, 0U,   0U,
        255U, 255U, 255U, 255U, 255U, 255U, 128U, 128U, 128U, 255U,
    };

    [[nodiscard]] auto makeMipmappedTexture() -> Renderer::TextureUploadData {
        return {
            .width = 2U,
            .height = 2U,
            .pixels = std::as_bytes(std::span{kMipmappedPixels}),
            .format = Renderer::TextureFormat::Rgba8Srgb,
            .mip_levels = 2U,
        };
    }

    template <typename Operation>
    [[nodiscard]] auto captureVulkanValidationOutput(Operation &&operation)
        -> std::pair<std::string, std::string> {
        testing::internal::CaptureStderr();

        std::string operation_error;

        try {
            std::forward<Operation>(operation)();
        } catch (const std::exception &exception) {
            operation_error = exception.what();
        } catch (...) {
            operation_error = "Unknown exception";
        }

        const std::string validation_output =
            testing::internal::GetCapturedStderr();

        return {validation_output, operation_error};
    }

    [[nodiscard]] auto
    hasUnexpectedVulkanValidationOutput(std::string_view output) -> bool {
        constexpr std::string_view kDiagnosticPrefix = "Vulkan validation:";
        constexpr std::string_view kSkippedDriverPrefix =
            " terminator_CreateInstance: Received return code ";
        constexpr std::string_view kSkippedDriverSuffix =
            "Skipping this driver.";

        std::size_t search_position{};

        while ((search_position =
                    output.find(kDiagnosticPrefix, search_position)) !=
               std::string_view::npos) {
            const std::size_t message_start =
                search_position + kDiagnosticPrefix.size();

            const std::size_t message_end = output.find('\n', message_start);

            const std::size_t message_length =
                message_end == std::string_view::npos
                    ? output.size() - message_start
                    : message_end - message_start;

            const std::string_view message =
                output.substr(message_start, message_length);

            const bool skipped_driver_warning =
                message.starts_with(kSkippedDriverPrefix) &&
                message.ends_with(kSkippedDriverSuffix);

            if (!skipped_driver_warning) {
                return true;
            }

            if (message_end == std::string_view::npos) {
                break;
            }

            search_position = message_end + 1U;
        }

        return false;
    }

    constexpr Renderer::DirectionalLightUniformData kTestLight{
        .direction =
            {
                .x = 0.0F,
                .y = 0.0F,
                .z = 1.0F,
                .w = 0.0F,
            },
        .color_intensity =
            {
                .x = 1.0F,
                .y = 1.0F,
                .z = 1.0F,
                .w = 1.0F,
            },
    };

    [[nodiscard]] auto makeLightBinding()
        -> Renderer::MaterialBufferBindingInput {
        return {
            .set = 0U,
            .binding = 1U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = std::as_bytes(std::span{&kTestLight, 1}),
        };
    }

} // namespace

class VulkanRendererTextureReplacementIntegrationTests
    : public ::testing::Test {
  protected:
    std::optional<Platform::GlfwContext> m_Glfw;
    std::optional<Platform::Window> m_Window;

    auto SetUp() -> void override {
        if (std::getenv("DISPLAY") == nullptr &&
            std::getenv("WAYLAND_DISPLAY") == nullptr) {
            GTEST_SKIP() << "Requires an available window-system display";
        }

        m_Glfw.emplace();
        m_Window.emplace(kWindowSize, "Signum Texture Replacement Test");

        Vulkan::VulkanInstance instance{
            "Signum Texture Replacement Test",
            false,
        };

        Vulkan::VulkanSurface surface{
            instance.nativeHandle(),
            m_Window->nativeHandle(),
        };

        const auto devices =
            Vulkan::enumeratePhysicalDevices(instance.nativeHandle());

        const auto discovered =
            Vulkan::inspectPhysicalDevices(devices, surface.nativeHandle());

        if (!Vulkan::selectPhysicalDevice(discovered).has_value()) {
            GTEST_SKIP() << "No suitable presentation-capable Vulkan device";
        }
    }
};

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesTextureAcrossRenderedFrames) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Texture replacement test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Texture Replacement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        true,
        initial_mesh,
        makeInitialTexture(),
        std::nullopt,
        makeGraphicsShaders(),
    };

    for (std::size_t i{}; i < 2U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceTexture(makeReplacementTexture()));

    for (std::size_t i{}; i < 2U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceTexture(makeThirdTexture()));

    for (std::size_t i{}; i < 3U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_NO_THROW(renderer.replaceTexture(makeMipmappedTexture()));

    for (std::size_t i{}; i < 2U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesShaderDrivenTextureAcrossRenderedFrames) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices, kTexturedVertexAttributes);

    if (!m_Window.has_value()) {
        FAIL() << "Shader-driven texture replacement test requires an "
                  "initialized window";
    }

    const std::array initial_bindings{makeLightBinding()};
    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Shader-Driven Texture Replacement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeInitialTexture(),
                std::nullopt,
                makeTexturedGraphicsShaders(),
                initial_bindings,
            };

            for (std::size_t i{}; i < 2U; ++i) {
                renderer.renderFrame();
            }

            renderer.replaceTexture(makeReplacementTexture());

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }

            renderer.replaceTexture(makeThirdTexture());

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }

            renderer.replaceTexture(makeMipmappedTexture());

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Texture replacement or rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesShaderDrivenTextureRepeatedlyBeforeRendering) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices, kTexturedVertexAttributes);

    if (!m_Window.has_value()) {
        FAIL() << "Repeated shader-driven texture replacement test requires "
                  "an initialized window";
    }

    constexpr std::size_t kFramesAfterReplacement = 5U;

    const std::array initial_bindings{makeLightBinding()};
    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Shader-Driven Texture Replacement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeInitialTexture(),
                std::nullopt,
                makeTexturedGraphicsShaders(),
                initial_bindings,
            };

            constexpr std::size_t kReplacementCount = 12U;

            for (std::size_t i{}; i < kReplacementCount; ++i) {
                if ((i % 2U) == 0U) {
                    renderer.replaceTexture(makeReplacementTexture());
                } else {
                    renderer.replaceTexture(makeThirdTexture());
                }
            }

            for (std::size_t i{}; i < kFramesAfterReplacement; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Repeated texture replacement or rendering failed: "
        << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       PreservesMaterialResourcesAcrossAlternatingReplacements) {
    const std::array model_matrices{Math::identity()};

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices, kMaterialTexturedVertexAttributes);

    constexpr std::array<float, 4> kInitialParameters{
        0.0F,
        0.0F,
        0.0F,
        0.0F,
    };

    constexpr std::array<float, 4> kReplacementParameters{
        1.0F,
        0.5F,
        0.0F,
        0.0F,
    };

    constexpr std::array<float, 4> kThirdParameters{
        0.5F,
        1.0F,
        0.0F,
        0.0F,
    };

    const std::array initial_bindings{
        Renderer::MaterialBufferBindingInput{
            .set = 1U,
            .binding = 2U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = std::as_bytes(std::span{kInitialParameters}),
        },
        makeLightBinding(),
    };

    Shader::GraphicsShaderData shaders = makeTexturedGraphicsShaders();

    shaders.vertex.artifact =
        Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                          "material_textured.vert.spv");

    if (!m_Window.has_value()) {
        FAIL() << "Combined material replacement test requires an "
                  "initialized window";
    }

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Combined Material Replacement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeInitialTexture(),
                std::nullopt,
                std::move(shaders),
                initial_bindings,
            };

            constexpr std::size_t kFrameCount{2U};

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }

            renderer.replaceTexture(makeReplacementTexture());

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }

            renderer.updateMaterialBuffer(
                1U, 2U, 0U, std::as_bytes(std::span{kReplacementParameters}));

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }

            renderer.replaceTexture(makeThirdTexture());

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }

            renderer.updateMaterialBuffer(
                1U, 2U, 0U, std::as_bytes(std::span{kThirdParameters}));

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Combined material replacement failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesTextureRepeatedlyWithoutInterveningFrames) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Repeated texture replacement test requires an initialized "
                  "window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Texture Replacement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        true,
        initial_mesh,
        makeInitialTexture(),
        std::nullopt,
        makeGraphicsShaders(),
    };

    constexpr std::size_t kReplacementCount = 12U;

    for (std::size_t i{}; i < kReplacementCount; ++i) {
        if ((i % 2U) == 0U) {
            ASSERT_NO_THROW(renderer.replaceTexture(makeReplacementTexture()));
        } else {
            ASSERT_NO_THROW(renderer.replaceTexture(makeThirdTexture()));
        }
    }

    for (std::size_t i{}; i < 3U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsReplacementWithoutExistingTexture) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Missing-texture replacement test requires an initialized "
                  "window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Texture Replacement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        makeGraphicsShaders(),
    };

    ASSERT_DEATH(
        { renderer.replaceTexture(makeReplacementTexture()); },
        "requires an existing texture binding");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RendersTwoGenericMaterialTexturesAcrossFrames) {
    if (!m_Window.has_value()) {
        FAIL() << "Multi-texture test requires an initialized window";
    }

    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh =
        makeInitialMesh(matrices, kTexturedVertexAttributes);

    const std::array buffer_bindings{makeLightBinding()};

    const std::array texture_bindings{
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .texture = makeInitialTexture(),
        },
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
            .texture = makeThirdTexture(),
        },
    };

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Multi-Texture Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                mesh,
                std::nullopt,
                std::nullopt,
                makeTwoTextureGraphicsShaders(),
                buffer_bindings,
                texture_bindings,
            };

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Multi-texture rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RendersLegacyAndGenericMaterialTexturesAcrossFrames) {
    if (!m_Window.has_value()) {
        FAIL() << "Mixed texture test requires an initialized window";
    }

    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh =
        makeInitialMesh(matrices, kTexturedVertexAttributes);

    const std::array buffer_bindings{makeLightBinding()};

    const std::array texture_bindings{
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
            .texture = makeThirdTexture(),
        },
    };

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Mixed Texture Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                mesh,
                makeInitialTexture(),
                std::nullopt,
                makeTwoTextureGraphicsShaders(),
                buffer_bindings,
                texture_bindings,
            };

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Mixed texture rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RendersTwoMaterialTexturesWithFrequentBufferUpdates) {
    if (!m_Window.has_value()) {
        FAIL() << "Frequent multi-texture test requires an "
                  "initialized window";
    }

    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh =
        makeInitialMesh(matrices, kTexturedVertexAttributes);

    Renderer::MaterialBufferBindingInput light_binding = makeLightBinding();

    light_binding.update_frequency =
        Renderer::MaterialBufferUpdateFrequency::Frequent;

    const std::array buffer_bindings{light_binding};

    const std::array texture_bindings{
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .texture = makeInitialTexture(),
        },
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 2U,
            .array_element = 0U,
            .texture = makeThirdTexture(),
        },
    };

    Renderer::DirectionalLightUniformData updated_light = kTestLight;
    constexpr float intensity = 0.5F;
    updated_light.color_intensity.w = intensity;

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Frequent Multi-Texture Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                mesh,
                std::nullopt,
                std::nullopt,
                makeTwoTextureGraphicsShaders(),
                buffer_bindings,
                texture_bindings,
            };

            for (std::size_t i{}; i < 3U; ++i) {
                renderer.renderFrame();
            }

            renderer.updateMaterialBuffer(
                0U, 1U, 0U, std::as_bytes(std::span{&updated_light, 1}));

            for (std::size_t i{}; i < 4U; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Frequent multi-texture rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsMissingFragmentShaderMaterialBinding) {
    if (!m_Window.has_value()) {
        FAIL() << "Material binding validation test requires an "
                  "initialized window";
    }

    Shader::GraphicsShaderData graphics_shaders = makeGraphicsShaders();

    graphics_shaders.fragment.artifact = Vulkan::loadSpirv(
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "textured.frag.spv");

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Material Binding Validation Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                std::nullopt, std::nullopt, std::move(graphics_shaders));
        },
        "initial material bindings do not satisfy fragment shader "
        "descriptor requirements");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsDuplicateInitialTextureBindings) {
    if (!m_Window.has_value()) {
        FAIL() << "Duplicate texture binding test requires an "
                  "initialized window";
    }

    const Renderer::MaterialTextureBindingInput binding{
        .set = 1U,
        .binding = 2U,
        .array_element = 0U,
        .texture = makeInitialTexture(),
    };

    const std::array bindings{binding, binding};

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Duplicate Texture Binding Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt, {}, bindings);
        },
        "initial material bindings contain duplicate descriptor locations");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsOverlappingInitialTextureAndBufferBindings) {
    if (!m_Window.has_value()) {
        FAIL() << "Overlapping material binding test requires an "
                  "initialized window";
    }

    const std::array buffer_bindings{makeLightBinding()};

    const std::array texture_bindings{
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 1U,
            .array_element = 0U,
            .texture = makeInitialTexture(),
        },
    };

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Overlapping Material Binding Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt, buffer_bindings,
                texture_bindings);
        },
        "initial material bindings contain duplicate descriptor locations");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsDuplicateInitialBufferBindings) {
    if (!m_Window.has_value()) {
        FAIL() << "Duplicate buffer binding test requires an "
                  "initialized window";
    }

    const std::array bindings{
        makeLightBinding(),
        makeLightBinding(),
    };

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Duplicate Buffer Binding Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                std::nullopt, std::nullopt, std::nullopt, bindings);
        },
        "initial material bindings contain duplicate descriptor locations");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsInitialTextureBindingConflictingWithLegacyTexture) {
    if (!m_Window.has_value()) {
        FAIL() << "Legacy texture collision test requires an "
                  "initialized window";
    }

    const std::array texture_bindings{
        Renderer::MaterialTextureBindingInput{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .texture = makeInitialTexture(),
        },
    };

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Legacy Texture Collision Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                makeInitialTexture(), std::nullopt, std::nullopt, {},
                texture_bindings);
        },
        "initial material bindings contain duplicate descriptor locations");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsInitialBufferBindingConflictingWithLegacyTexture) {
    if (!m_Window.has_value()) {
        FAIL() << "Legacy texture and buffer collision test requires an "
                  "initialized window";
    }

    Renderer::MaterialBufferBindingInput conflicting_binding =
        makeLightBinding();

    conflicting_binding.binding = 0U;

    const std::array buffer_bindings{
        conflicting_binding,
    };

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Legacy Texture Buffer Collision Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                makeInitialTexture(), std::nullopt, std::nullopt,
                buffer_bindings);
        },
        "initial material bindings contain duplicate descriptor locations");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       AcceptsCompatibleFragmentShaderMaterialBinding) {
    if (!m_Window.has_value()) {
        FAIL() << "Material binding validation test requires an "
                  "initialized window";
    }

    Shader::GraphicsShaderData graphics_shaders = makeGraphicsShaders();

    graphics_shaders.fragment.artifact = Vulkan::loadSpirv(
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "textured.frag.spv");

    ASSERT_NO_THROW({
        Vulkan::VulkanRenderer renderer(
            "Signum Material Binding Validation Test", m_Window.value(),
            Renderer::PresentationPreference::VSync, false, std::nullopt,
            makeInitialTexture(), std::nullopt, std::move(graphics_shaders));
    });
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesFragmentShaderAcrossRenderedFrames) {
    if (!m_Window.has_value()) {
        FAIL() << "Fragment shader replacement test requires an "
                  "initialized window";
    }

    const std::array model_matrices{Math::identity()};

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices, kMaterialTexturedVertexAttributes);

    constexpr std::array<float, 4> kMaterialParameters{
        1.0F,
        1.0F,
        1.0F,
        1.0F,
    };

    const std::array material_bindings{
        Renderer::MaterialBufferBindingInput{
            .set = 1U,
            .binding = 2U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = std::as_bytes(std::span{kMaterialParameters}),
        },
        Renderer::MaterialBufferBindingInput{
            .set = 1U,
            .binding = 3U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = std::as_bytes(std::span{kMaterialParameters}),
        },
    };

    Shader::GraphicsShaderData shaders = makeGraphicsShaders();

    shaders.vertex.artifact =
        Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                          "material_textured.vert.spv");

    shaders.fragment.artifact =
        Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                          "material_mixed.frag.spv");

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Fragment Shader Replacement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                std::nullopt,
                std::nullopt,
                std::move(shaders),
                material_bindings,
            };

            for (std::size_t i{}; i < 2U; ++i) {
                renderer.renderFrame();
            }

            Shader::ShaderStageInput replacement_fragment{
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "triangle.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            };

            renderer.replaceFragmentShader(std::move(replacement_fragment));

            for (std::size_t i{}; i < 2U; ++i) {
                renderer.renderFrame();
            }

            Shader::ShaderStageInput restored_fragment{
                .artifact = Vulkan::loadSpirv(
                    std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                    "material_mixed.frag.spv"),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            };

            renderer.replaceFragmentShader(std::move(restored_fragment));

            for (std::size_t i{}; i < 2U; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Fragment shader replacement failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       PreservesActiveShaderWhenReplacementBindingsAreIncompatible) {
    if (!m_Window.has_value()) {
        FAIL() << "Fragment shader compatibility test requires an "
                  "initialized window";
    }

    const std::array model_matrices{Math::identity()};
    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    std::optional<Core::Error::Code> replacement_error_code;
    std::string replacement_error_message;
    std::string replacement_operation;

    const auto [validation_output,
                operation_error] = captureVulkanValidationOutput([&] -> void {
        Vulkan::VulkanRenderer renderer{
            "Signum Rejected Fragment Shader Replacement Test",
            m_Window.value(),
            Renderer::PresentationPreference::VSync,
            true,
            initial_mesh,
            std::nullopt,
            std::nullopt,
            makeGraphicsShaders(),
        };

        for (std::size_t i{}; i < 2U; ++i) {
            renderer.renderFrame();
        }

        Shader::ShaderStageInput incompatible_fragment{
            .artifact = Vulkan::loadSpirv(
                std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} /
                "textured.frag.spv"),
            .selection =
                {
                    .stage = Shader::ShaderStage::Fragment,
                    .entry_point = "main",
                },
        };

        try {
            renderer.replaceFragmentShader(std::move(incompatible_fragment));
        } catch (const Core::Error::EngineError &error) {
            replacement_error_code = error.getCode();
            replacement_error_message = error.what();
            replacement_operation = error.getOperation();
        }

        for (std::size_t i{}; i < 3U; ++i) {
            renderer.renderFrame();
        }
    });

    EXPECT_TRUE(operation_error.empty())
        << "Rendering after rejected shader replacement failed: "
        << operation_error;

    ASSERT_TRUE(replacement_error_code.has_value());

    if (replacement_error_code.has_value()) {
        EXPECT_EQ(*replacement_error_code,
                  Core::Error::Code::VulkanMaterialBindingsIncompatible);
    } else {
        FAIL() << "Expected the replacement operation to provide an error code";
    }

    EXPECT_EQ(replacement_error_message,
              "Replacement fragment shader requires material resource bindings "
              "that are not configured");

    EXPECT_EQ(replacement_operation, "Replace Vulkan Fragment Shader");

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RejectsIncompatibleVertexShaderMaterialBinding) {
    if (!m_Window.has_value()) {
        FAIL() << "Vertex material binding validation test requires "
                  "an initialized window";
    }

    Shader::GraphicsShaderData graphics_shaders = makeGraphicsShaders();

    graphics_shaders.vertex.artifact =
        Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} /
                          "material_uniform.vert.spv");

    ASSERT_DEATH(
        {
            Vulkan::VulkanRenderer renderer(
                "Signum Vertex Material Binding Test", m_Window.value(),
                Renderer::PresentationPreference::VSync, false, std::nullopt,
                makeInitialTexture(), std::nullopt,
                std::move(graphics_shaders));
        },
        "initial material bindings do not satisfy vertex shader "
        "descriptor requirements");
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RetiresShaderDrivenTextureAndDescriptorsBeforeShutdown) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices, kTexturedVertexAttributes);

    if (!m_Window.has_value()) {
        FAIL() << "Shader-driven texture retirement test requires an "
                  "initialized window";
    }

    const std::array initial_bindings{makeLightBinding()};
    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Shader-Driven Texture Retirement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeInitialTexture(),
                std::nullopt,
                makeTexturedGraphicsShaders(),
                initial_bindings,
            };

            renderer.renderFrame();
            renderer.replaceTexture(makeReplacementTexture());
        });

    EXPECT_TRUE(operation_error.empty())
        << "Shader-driven texture replacement or shutdown failed: "
        << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       RetiresTextureBindingBeforeShutdown) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Texture binding retirement test requires an initialized "
                  "window";
    }

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Texture Binding Retirement Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeInitialTexture(),
                std::nullopt,
                makeGraphicsShaders(),
            };

            renderer.renderFrame();
            renderer.replaceTexture(makeReplacementTexture());
        });

    EXPECT_TRUE(operation_error.empty())
        << "Texture replacement or shutdown failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}
