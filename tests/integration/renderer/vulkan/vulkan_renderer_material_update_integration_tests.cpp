#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
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

namespace Math = SNE::Engine::Core::Math;
namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kMaterialSet{1U};
    constexpr std::uint32_t kMaterialBinding{2U};
    constexpr std::uint32_t kArrayElement{0U};

    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    struct TestVertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
    };

    static_assert(std::is_standard_layout_v<TestVertex>);

    constexpr std::array kVertices{
        TestVertex{
            .position = {-0.5F, -0.5F, 0.0F},
            .color = {1.0F, 0.0F, 0.0F},
        },
        TestVertex{
            .position = {0.5F, -0.5F, 0.0F},
            .color = {0.0F, 1.0F, 0.0F},
        },
        TestVertex{
            .position = {0.0F, 0.5F, 0.0F},
            .color = {0.0F, 0.0F, 1.0F},
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
    };

    constexpr std::array<float, 4> kInitialParameters{
        1.0F,
        1.0F,
        1.0F,
        1.0F,
    };

    constexpr std::array<float, 4> kReplacementParameters{
        1.0F,
        0.0F,
        0.0F,
        1.0F,
    };

    constexpr std::array<float, 4> kThirdParameters{
        0.0F,
        1.0F,
        0.0F,
        1.0F,
    };

    [[nodiscard]] auto parameterBytes(std::span<const float> parameters)
        -> std::span<const std::byte> {
        return std::as_bytes(parameters);
    }

    [[nodiscard]] auto initialBinding()
        -> Renderer::MaterialBufferBindingInput {
        return {
            .set = kMaterialSet,
            .binding = kMaterialBinding,
            .array_element = kArrayElement,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = parameterBytes(kInitialParameters),
        };
    }

    [[nodiscard]] auto initialStorageBinding()
        -> Renderer::MaterialBufferBindingInput {
        Renderer::MaterialBufferBindingInput input = initialBinding();
        input.type = Shader::ShaderResourceType::StorageBuffer;
        return input;
    }

    [[nodiscard]] auto makeInitialMesh(std::span<const Math::Matrix4f> matrices)
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
            .model_matrices = matrices,
        };
    }

    [[nodiscard]] auto makeMaterialShaders() -> Shader::GraphicsShaderData {
        return {
            .vertex =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        "material_update.vert.spv"),
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

    [[nodiscard]] auto makeStorageMaterialShaders()
        -> Shader::GraphicsShaderData {
        Shader::GraphicsShaderData shaders = makeMaterialShaders();

        shaders.vertex.artifact =
            Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                              "material_storage.vert.spv");

        return shaders;
    }

    [[nodiscard]] auto makeTexturedShaders() -> Shader::GraphicsShaderData {
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

    constexpr std::array<std::uint8_t, 4> kTexturePixels{
        255U,
        255U,
        255U,
        255U,
    };

    [[nodiscard]] auto makeTexture() -> Renderer::TextureUploadData {
        return {
            .width = 1U,
            .height = 1U,
            .pixels = std::as_bytes(std::span{kTexturePixels}),
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

            if (!(message.starts_with(kSkippedDriverPrefix) &&
                  message.ends_with(kSkippedDriverSuffix))) {
                return true;
            }

            if (message_end == std::string_view::npos) {
                break;
            }

            search_position = message_end + 1U;
        }

        return false;
    }
} // namespace

class VulkanRendererMaterialUpdateIntegrationTests : public ::testing::Test {
  protected:
    std::optional<Platform::GlfwContext> m_Glfw;
    std::optional<Platform::Window> m_Window;

    auto SetUp() -> void override {
        if (std::getenv("DISPLAY") == nullptr &&
            std::getenv("WAYLAND_DISPLAY") == nullptr) {
            GTEST_SKIP() << "Requires an available window-system display";
        }

        m_Glfw.emplace();
        m_Window.emplace(kWindowSize, "Signum Material Update Test");

        Vulkan::VulkanInstance instance{
            "Signum Material Update Test",
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

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       UpdatesUniformBufferAcrossRenderedFrames) {
    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh = makeInitialMesh(matrices);
    const std::array initial_bindings{initialBinding()};

    const auto [validation_output,
                operation_error] = captureVulkanValidationOutput([&] -> void {
        Vulkan::VulkanRenderer renderer{
            "Signum Material Update Test",
            m_Window.value(),
            Renderer::PresentationPreference::VSync,
            true,
            mesh,
            std::nullopt,
            std::nullopt,
            makeMaterialShaders(),
            initial_bindings,
        };

        constexpr std::size_t kFramesBefore{2U};
        constexpr std::size_t kFramesAfter{4U};

        for (std::size_t i{}; i < kFramesBefore; ++i) {
            renderer.renderFrame();
        }

        renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                      kArrayElement,
                                      parameterBytes(kReplacementParameters));

        for (std::size_t i{}; i < kFramesAfter; ++i) {
            renderer.renderFrame();
        }

        renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                      kArrayElement,
                                      parameterBytes(kThirdParameters));

        for (std::size_t i{}; i < kFramesAfter; ++i) {
            renderer.renderFrame();
        }
    });

    EXPECT_TRUE(operation_error.empty())
        << "Material update or rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       UpdatesUniformBufferRepeatedlyBeforeRendering) {
    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh = makeInitialMesh(matrices);
    const std::array initial_bindings{initialBinding()};

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Repeated Material Update Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                mesh,
                std::nullopt,
                std::nullopt,
                makeMaterialShaders(),
                initial_bindings,
            };

            constexpr std::size_t kUpdateCount{12U};

            for (std::size_t i{}; i < kUpdateCount; ++i) {
                const auto parameters =
                    (i % 2U) == 0U ? parameterBytes(kReplacementParameters)
                                   : parameterBytes(kThirdParameters);

                renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                              kArrayElement, parameters);
            }

            constexpr std::size_t kFrameCount{4U};

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Repeated material update failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RetiresUpdatedUniformBufferBeforeShutdown) {
    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh = makeInitialMesh(matrices);
    const std::array initial_bindings{initialBinding()};

    const auto [validation_output,
                operation_error] = captureVulkanValidationOutput([&] -> void {
        Vulkan::VulkanRenderer renderer{
            "Signum Material Retirement Test",
            m_Window.value(),
            Renderer::PresentationPreference::VSync,
            true,
            mesh,
            std::nullopt,
            std::nullopt,
            makeMaterialShaders(),
            initial_bindings,
        };

        renderer.renderFrame();

        renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                      kArrayElement,
                                      parameterBytes(kReplacementParameters));
    });

    EXPECT_TRUE(operation_error.empty())
        << "Material replacement or shutdown failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RejectsUpdateWithoutMaterialDescriptors) {
    if (!m_Window.has_value()) {
        FAIL() << "Missing-material test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Missing Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
    };

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                kMaterialSet, kMaterialBinding, kArrayElement,
                parameterBytes(kReplacementParameters));
        },
        "requires allocated material descriptor resources");
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RejectsInvalidUniformBufferUpdates) {
    const std::array initial_bindings{initialBinding()};

    if (!m_Window.has_value()) {
        FAIL() << "Material validation test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Material Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeMaterialShaders(),
        initial_bindings,
    };

    const std::span<const std::byte> empty_data{};

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                          kArrayElement, empty_data);
        },
        "requires nonempty data");

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                0U, kMaterialBinding, kArrayElement,
                parameterBytes(kReplacementParameters));
        },
        "requires an existing material binding");

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                kMaterialSet, 0U, kArrayElement,
                parameterBytes(kReplacementParameters));
        },
        "requires an existing material binding");

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                kMaterialSet, kMaterialBinding, 1U,
                parameterBytes(kReplacementParameters));
        },
        "requires an existing material binding");

    constexpr std::array<float, 2> kWrongSizeParameters{
        1.0F,
        0.0F,
    };

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(kMaterialSet, kMaterialBinding,
                                          kArrayElement,
                                          parameterBytes(kWrongSizeParameters));
        },
        "requires data matching the existing buffer size");
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RejectsNonUniformMaterialBinding) {
    if (!m_Window.has_value()) {
        FAIL() << "Nonuniform binding test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Nonuniform Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        makeTexture(),
        std::nullopt,
        makeTexturedShaders(),
    };

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                0U, 0U, 0U, parameterBytes(kReplacementParameters));
        },
        "requires a uniform-buffer binding");
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RendersWithStorageBufferMaterial) {
    const std::array matrices{Math::identity()};
    const Renderer::InitialMeshData mesh = makeInitialMesh(matrices);
    const std::array initial_bindings{initialStorageBinding()};

    if (!m_Window.has_value()) {
        FAIL() << "Missing-material test requires an initialized window";
    }

    const auto [validation_output, operation_error] =
        captureVulkanValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Storage Buffer Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                mesh,
                std::nullopt,
                std::nullopt,
                makeStorageMaterialShaders(),
                initial_bindings,
            };

            constexpr std::size_t kFrameCount{4U};

            for (std::size_t i{}; i < kFrameCount; ++i) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Storage-buffer material rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedVulkanValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}

TEST_F(VulkanRendererMaterialUpdateIntegrationTests,
       RejectsUniformUpdateForStorageBufferBinding) {
    const std::array initial_bindings{initialStorageBinding()};

    if (!m_Window.has_value()) {
        FAIL() << "Missing-material test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Storage Buffer Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeStorageMaterialShaders(),
        initial_bindings,
    };

    ASSERT_DEATH(
        {
            renderer.updateMaterialBuffer(
                kMaterialSet, kMaterialBinding, kArrayElement,
                parameterBytes(kReplacementParameters));
        },
        "requires a uniform-buffer binding");
}
