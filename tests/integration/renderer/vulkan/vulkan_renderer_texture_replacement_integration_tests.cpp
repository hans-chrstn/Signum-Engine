#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
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
#include <type_traits>
#include <utility>

namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct TestVertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
        std::array<float, 2> uv;
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
            .uv = {0.0F, 0.0F},
        },
        TestVertex{
            .position = {0.5F, -0.5F, 0.0F},
            .color = {0.0F, 1.0F, 0.0F},
            .uv = {1.0F, 0.0F},
        },
        TestVertex{
            .position = {0.0F, 0.5F, 0.0F},
            .color = {0.0F, 0.0F, 1.0F},
            .uv = {0.5F, 1.0F},
        },
    };

    constexpr std::array<std::uint16_t, 3> kIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kVertexAttributes{
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

    [[nodiscard]] auto
    makeInitialMesh(std::span<const Math::Matrix4f> model_matrices)
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
                    .attributes = kVertexAttributes,
                },
            .model_matrices = model_matrices,
        };
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
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Shader-driven texture replacement test requires an "
                  "initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Shader-Driven Texture Replacement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        true,
        initial_mesh,
        makeInitialTexture(),
        std::nullopt,
        makeTexturedGraphicsShaders(),
    };

    for (std::size_t i{}; i < 2U; ++i) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
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

    EXPECT_EQ(validation_output.find("Vulkan validation:"), std::string::npos)
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererTextureReplacementIntegrationTests,
       ReplacesShaderDrivenTextureRepeatedlyBeforeRendering) {
    const std::array model_matrices{
        Math::identity(),
    };

    const Renderer::InitialMeshData initial_mesh =
        makeInitialMesh(model_matrices);

    if (!m_Window.has_value()) {
        FAIL() << "Repeated shader-driven texture replacement test requires "
                  "an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Shader-Driven Texture Replacement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        true,
        initial_mesh,
        makeInitialTexture(),
        std::nullopt,
        makeTexturedGraphicsShaders(),
    };

    constexpr std::size_t val = 5U;
    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            constexpr std::size_t kReplacementCount = 12U;

            for (std::size_t i{}; i < kReplacementCount; ++i) {
                if ((i % 2U) == 0U) {
                    renderer.replaceTexture(makeReplacementTexture());
                } else {
                    renderer.replaceTexture(makeThirdTexture());
                }
            }

            for (std::size_t i{}; i < val; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Repeated texture replacement or rendering failed: "
        << operation_error;

    EXPECT_EQ(validation_output.find("Vulkan validation:"), std::string::npos)
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
