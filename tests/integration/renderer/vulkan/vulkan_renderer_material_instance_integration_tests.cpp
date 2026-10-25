#include "engine/core/math/matrix4.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_instance_description.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
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
#include <utility>

namespace Platform = SNE::Engine::Platform;
namespace Renderer = SNE::Engine::Renderer;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr Platform::WindowSize kWindowSize{
        .width = 640,
        .height = 480,
    };

    constexpr std::uint32_t kMaterialSet{1U};
    constexpr std::uint32_t kMaterialBinding{2U};
    constexpr std::uint32_t kArrayElement{0U};

    constexpr std::array<float, 4> kBufferData{
        1.0F,
        1.0F,
        1.0F,
        1.0F,
    };

    [[nodiscard]] auto makeShaders(const char *vertex_filename)
        -> Shader::GraphicsShaderData {
        return {
            .vertex =
                {
                    .artifact = Vulkan::loadSpirv(
                        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                        vertex_filename),
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

    [[nodiscard]] auto makeInitialBufferBinding()
        -> Renderer::MaterialBufferBindingInput {
        return {
            .set = kMaterialSet,
            .binding = kMaterialBinding,
            .array_element = kArrayElement,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .data = std::as_bytes(std::span{kBufferData}),
        };
    }

    [[nodiscard]] auto
    makeBufferBinding(std::uint32_t binding_index = kMaterialBinding)
        -> Renderer::MaterialResourceBinding {
        return {
            .set = kMaterialSet,
            .binding = binding_index,
            .array_element = kArrayElement,
            .type = Shader::ShaderResourceType::UniformBuffer,
            .resource_handle = {},
        };
    }

    constexpr std::array<std::uint8_t, 4> kFirstTexturePixels{
        255U,
        0U,
        0U,
        255U,
    };

    constexpr std::array<std::uint8_t, 4> kSecondTexturePixels{
        0U,
        0U,
        255U,
        255U,
    };

    [[nodiscard]] auto makeTexture(std::span<const std::uint8_t> pixels)
        -> Renderer::TextureUploadData {
        return {
            .width = 1U,
            .height = 1U,
            .pixels = std::as_bytes(pixels),
        };
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
                        "material_instance_textured.frag.spv"),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };
    }

    [[nodiscard]] auto
    makeTextureMaterialDescription(Renderer::GpuResourceHandle texture_handle)
        -> Renderer::MaterialInstanceDescription {

        Renderer::MaterialInstanceDescription description{};

        description.resource_bindings.bindings.push_back(
            Renderer::MaterialResourceBinding{
                .set = 0U,
                .binding = 0U,
                .array_element = 0U,
                .type = Shader::ShaderResourceType::CombinedImageSampler,
                .resource_handle = texture_handle,
            });

        return description;
    }

    [[nodiscard]] auto
    makeBufferMaterialDescription(Renderer::GpuResourceHandle buffer_handle,
                                  Shader::ShaderResourceType type)
        -> Renderer::MaterialInstanceDescription {

        Renderer::MaterialInstanceDescription description{};

        Renderer::MaterialResourceBinding binding = makeBufferBinding();
        binding.type = type;
        binding.resource_handle = buffer_handle;

        description.resource_bindings.bindings.push_back(binding);

        return description;
    }

    struct TestVertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
    };

    constexpr std::array kMeshVertices{
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

    constexpr std::array<std::uint16_t, 3> kMeshIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kMeshAttributes{
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

    [[nodiscard]] auto makeInitialMesh() -> Renderer::InitialMeshData {
        return {
            .mesh =
                {
                    .vertex_bytes = std::as_bytes(std::span{kMeshVertices}),
                    .vertex_stride =
                        static_cast<std::uint32_t>(sizeof(TestVertex)),
                    .indices = kMeshIndices,
                },
            .layout =
                {
                    .attributes = kMeshAttributes,
                },
            .model_matrices = {},
        };
    }

    [[nodiscard]] auto makeDrawInstance(
        Renderer::GpuResourceHandle mesh_handle,
        std::optional<Renderer::GpuResourceHandle> material_handle =
            std::nullopt) -> Renderer::MeshDrawInstance {

        Renderer::MeshDrawInstance instance{
            .mesh_handle = mesh_handle,
            .model_matrix = Math::identity(),
            .material_handle = {},
        };

        instance.material_handle = material_handle;

        return instance;
    }

    struct MaterialDrawVertex {
        std::array<float, 3> position;
        std::array<float, 3> normal;
        std::array<float, 2> uv;
        std::array<float, 4> tangent;
    };

    constexpr std::array kMaterialDrawVertices{
        MaterialDrawVertex{
            .position = {-0.5F, -0.5F, 0.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {0.0F, 0.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
        MaterialDrawVertex{
            .position = {0.5F, -0.5F, 0.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {1.0F, 0.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
        MaterialDrawVertex{
            .position = {0.0F, 0.5F, 0.0F},
            .normal = {0.0F, 0.0F, 1.0F},
            .uv = {0.5F, 1.0F},
            .tangent = {1.0F, 0.0F, 0.0F, 1.0F},
        },
    };

    constexpr std::array<std::uint16_t, 3> kMaterialDrawIndices{
        0U,
        1U,
        2U,
    };

    constexpr std::array kMaterialDrawAttributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(
                offsetof(MaterialDrawVertex, position)),
        },
        Renderer::VertexAttributeDescription{
            .location = 1U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = static_cast<std::uint32_t>(
                offsetof(MaterialDrawVertex, normal)),
        },
        Renderer::VertexAttributeDescription{
            .location = 2U,
            .format = Renderer::VertexFormat::Float32x2,
            .offset =
                static_cast<std::uint32_t>(offsetof(MaterialDrawVertex, uv)),
        },
        Renderer::VertexAttributeDescription{
            .location = 3U,
            .format = Renderer::VertexFormat::Float32x4,
            .offset = static_cast<std::uint32_t>(
                offsetof(MaterialDrawVertex, tangent)),
        },
    };

    [[nodiscard]] auto makeTexturedInitialMesh() -> Renderer::InitialMeshData {
        return {
            .mesh =
                {
                    .vertex_bytes =
                        std::as_bytes(std::span{kMaterialDrawVertices}),
                    .vertex_stride =
                        static_cast<std::uint32_t>(sizeof(MaterialDrawVertex)),
                    .indices = kMaterialDrawIndices,
                },
            .layout =
                {
                    .attributes = kMaterialDrawAttributes,
                },
            .model_matrices = {},
        };
    }

    template <typename Operation>
    [[nodiscard]] auto
    captureMixedMaterialValidationOutput(Operation &&operation)
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
    hasUnexpectedMixedMaterialValidationOutput(std::string_view output)
        -> bool {
        constexpr std::string_view kPrefix = "Vulkan validation:";
        constexpr std::string_view kSkippedDriverPrefix =
            " terminator_CreateInstance: Received return code ";
        constexpr std::string_view kSkippedDriverSuffix =
            "Skipping this driver.";

        std::size_t position{};

        while ((position = output.find(kPrefix, position)) !=
               std::string_view::npos) {
            const std::size_t message_start = position + kPrefix.size();

            const std::size_t message_end = output.find('\n', message_start);

            const std::string_view message = output.substr(
                message_start, message_end == std::string_view::npos
                                   ? std::string_view::npos
                                   : message_end - message_start);

            if (!(message.starts_with(kSkippedDriverPrefix) &&
                  message.ends_with(kSkippedDriverSuffix))) {
                return true;
            }

            if (message_end == std::string_view::npos) {
                break;
            }

            position = message_end + 1U;
        }

        return false;
    }

} // namespace

class VulkanRendererMaterialInstanceIntegrationTests : public ::testing::Test {
  protected:
    std::optional<Platform::GlfwContext> m_Glfw;
    std::optional<Platform::Window> m_Window;

    auto SetUp() -> void override {
        if (std::getenv("DISPLAY") == nullptr &&
            std::getenv("WAYLAND_DISPLAY") == nullptr) {
            GTEST_SKIP() << "Requires an available window-system display";
        }

        m_Glfw.emplace();
        m_Window.emplace(kWindowSize, "Signum Material Instance Test");

        Vulkan::VulkanInstance instance{
            "Signum Material Instance Test",
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

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsRegistrationWithoutInitializedMaterial) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }
    Vulkan::VulkanRenderer renderer{
        "Signum Missing Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
    };

    const Renderer::MaterialInstanceDescription description{};

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "requires an initialized material");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RegistersIndependentInstancesWithoutDescriptors) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }
    Vulkan::VulkanRenderer renderer{
        "Signum Independent Material Instance Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::MaterialInstanceDescription description{};

    const Renderer::GpuResourceHandle first =
        renderer.registerMaterialInstance(description);

    const Renderer::GpuResourceHandle second =
        renderer.registerMaterialInstance(description);

    EXPECT_NE(first, second);
    EXPECT_NE(first.registry_id, 0U);
    EXPECT_EQ(first.registry_id, second.registry_id);
    EXPECT_NE(first.generation, 0U);
    EXPECT_NE(second.generation, 0U);

    EXPECT_NO_THROW(renderer.renderFrame());
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsBindingsWhenNoDescriptorLayoutsExist) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }
    Vulkan::VulkanRenderer renderer{
        "Signum Descriptor-Free Material Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    Renderer::MaterialInstanceDescription description{};
    description.resource_bindings.bindings.push_back(makeBufferBinding());

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "requires empty bindings when no descriptor layouts exist");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsMissingShaderBindings) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }
    const std::array initial_bindings{makeInitialBufferBinding()};

    Vulkan::VulkanRenderer renderer{
        "Signum Missing Binding Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_update.vert.spv"),
        initial_bindings,
    };

    const Renderer::MaterialInstanceDescription description{};

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "requires bindings compatible with the active shaders");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsUnregisteredBufferHandles) {
    const std::array initial_bindings{makeInitialBufferBinding()};

    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Invalid Material Buffer Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_update.vert.spv"),
        initial_bindings,
    };

    Renderer::MaterialInstanceDescription description{};
    description.resource_bindings.bindings.push_back(makeBufferBinding());

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "requires registered buffer handles");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsUndeclaredDescriptorBindings) {
    const std::array initial_bindings{makeInitialBufferBinding()};

    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Undeclared Material Binding Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_update.vert.spv"),
        initial_bindings,
    };

    Renderer::MaterialInstanceDescription description{};

    description.resource_bindings.bindings.push_back(makeBufferBinding(3U));
    description.resource_bindings.bindings.push_back(makeBufferBinding());

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "material binding is not declared in");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RegistersIndependentTextureBackedMaterialInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Texture Material Instance Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        makeTexture(kFirstTexturePixels),
        std::nullopt,
        makeTexturedShaders(),
    };

    const Renderer::GpuResourceHandle first_texture =
        renderer.registerTexture(makeTexture(kFirstTexturePixels));

    const Renderer::GpuResourceHandle second_texture =
        renderer.registerTexture(makeTexture(kSecondTexturePixels));

    EXPECT_NE(first_texture, second_texture);
    EXPECT_EQ(first_texture.registry_id, second_texture.registry_id);

    const Renderer::MaterialInstanceDescription first_description =
        makeTextureMaterialDescription(first_texture);

    const Renderer::MaterialInstanceDescription second_description =
        makeTextureMaterialDescription(second_texture);

    const Renderer::GpuResourceHandle first_material =
        renderer.registerMaterialInstance(first_description);

    const Renderer::GpuResourceHandle second_material =
        renderer.registerMaterialInstance(second_description);

    EXPECT_NE(first_material, second_material);
    EXPECT_EQ(first_material.registry_id, second_material.registry_id);
    EXPECT_NE(first_material.registry_id, first_texture.registry_id);

    constexpr std::size_t kFrameCount{3U};

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        EXPECT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsInvalidTextureHandlesInMaterialInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Invalid Texture Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        makeTexture(kFirstTexturePixels),
        std::nullopt,
        makeTexturedShaders(),
    };

    const Renderer::MaterialInstanceDescription invalid_description =
        makeTextureMaterialDescription({});

    ASSERT_DEATH(
        {
            static_cast<void>(
                renderer.registerMaterialInstance(invalid_description));
        },
        "requires registered texture handles");

    const Renderer::GpuResourceHandle texture =
        renderer.registerTexture(makeTexture(kSecondTexturePixels));

    const Renderer::MaterialInstanceDescription valid_description =
        makeTextureMaterialDescription(texture);

    const Renderer::GpuResourceHandle material =
        renderer.registerMaterialInstance(valid_description);

    const Renderer::MaterialInstanceDescription foreign_description =
        makeTextureMaterialDescription(material);

    ASSERT_DEATH(
        {
            static_cast<void>(
                renderer.registerMaterialInstance(foreign_description));
        },
        "requires registered texture handles");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsUndeclaredBindingsWithValidTextureResources) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Undeclared Texture Binding Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        makeTexture(kFirstTexturePixels),
        std::nullopt,
        makeTexturedShaders(),
    };

    const Renderer::GpuResourceHandle texture =
        renderer.registerTexture(makeTexture(kSecondTexturePixels));

    Renderer::MaterialInstanceDescription description =
        makeTextureMaterialDescription(texture);

    Renderer::MaterialResourceBinding undeclared_binding =
        description.resource_bindings.bindings.front();

    constexpr std::uint32_t kBindingSize = 5U;

    undeclared_binding.binding = kBindingSize;

    description.resource_bindings.bindings.push_back(undeclared_binding);

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "material binding is not declared in");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RegistersIndependentUniformBufferMaterialInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const std::array initial_bindings{makeInitialBufferBinding()};

    Vulkan::VulkanRenderer renderer{
        "Signum Uniform Material Instance Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_update.vert.spv"),
        initial_bindings,
    };

    const std::span<const std::byte> data =
        std::as_bytes(std::span{kBufferData});

    const Renderer::GpuResourceHandle first_buffer =
        renderer.registerMaterialBuffer(
            data, Shader::ShaderResourceType::UniformBuffer);

    const Renderer::GpuResourceHandle second_buffer =
        renderer.registerMaterialBuffer(
            data, Shader::ShaderResourceType::UniformBuffer);

    EXPECT_NE(first_buffer, second_buffer);

    const Renderer::GpuResourceHandle first_material =
        renderer.registerMaterialInstance(makeBufferMaterialDescription(
            first_buffer, Shader::ShaderResourceType::UniformBuffer));

    const Renderer::GpuResourceHandle second_material =
        renderer.registerMaterialInstance(makeBufferMaterialDescription(
            second_buffer, Shader::ShaderResourceType::UniformBuffer));

    EXPECT_NE(first_material, second_material);
    EXPECT_EQ(first_material.registry_id, second_material.registry_id);
    EXPECT_NE(first_material.registry_id, first_buffer.registry_id);

    constexpr std::size_t kFrameCount{3U};

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        EXPECT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RegistersIndependentStorageBufferMaterialInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Renderer::MaterialBufferBindingInput initial_binding =
        makeInitialBufferBinding();

    initial_binding.type = Shader::ShaderResourceType::StorageBuffer;

    const std::array initial_bindings{initial_binding};

    Vulkan::VulkanRenderer renderer{
        "Signum Storage Material Instance Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_storage.vert.spv"),
        initial_bindings,
    };

    const std::span<const std::byte> data =
        std::as_bytes(std::span{kBufferData});

    const Renderer::GpuResourceHandle first_buffer =
        renderer.registerMaterialBuffer(
            data, Shader::ShaderResourceType::StorageBuffer);

    const Renderer::GpuResourceHandle second_buffer =
        renderer.registerMaterialBuffer(
            data, Shader::ShaderResourceType::StorageBuffer);

    EXPECT_NE(first_buffer, second_buffer);

    const Renderer::GpuResourceHandle first_material =
        renderer.registerMaterialInstance(makeBufferMaterialDescription(
            first_buffer, Shader::ShaderResourceType::StorageBuffer));

    const Renderer::GpuResourceHandle second_material =
        renderer.registerMaterialInstance(makeBufferMaterialDescription(
            second_buffer, Shader::ShaderResourceType::StorageBuffer));

    EXPECT_NE(first_material, second_material);
    EXPECT_EQ(first_material.registry_id, second_material.registry_id);

    constexpr std::size_t kFrameCount{3U};

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        EXPECT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsBufferUsageIncompatibleWithShaderBinding) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const std::array initial_bindings{makeInitialBufferBinding()};

    Vulkan::VulkanRenderer renderer{
        "Signum Material Buffer Usage Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("material_update.vert.spv"),
        initial_bindings,
    };

    const Renderer::GpuResourceHandle storage_buffer =
        renderer.registerMaterialBuffer(
            std::as_bytes(std::span{kBufferData}),
            Shader::ShaderResourceType::StorageBuffer);

    const Renderer::MaterialInstanceDescription description =
        makeBufferMaterialDescription(
            storage_buffer, Shader::ShaderResourceType::UniformBuffer);

    ASSERT_DEATH(
        { static_cast<void>(renderer.registerMaterialInstance(description)); },
        "requires buffer usage compatible with the binding type");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsInvalidIndependentBufferRegistration) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Independent Buffer Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
    };

    const std::span<const std::byte> empty_data{};

    ASSERT_DEATH(
        {
            static_cast<void>(renderer.registerMaterialBuffer(
                empty_data, Shader::ShaderResourceType::UniformBuffer));
        },
        "material buffer registration requires nonempty data");

    ASSERT_DEATH(
        {
            static_cast<void>(renderer.registerMaterialBuffer(
                std::as_bytes(std::span{kBufferData}),
                Shader::ShaderResourceType::CombinedImageSampler));
        },
        "requires a uniform or storage buffer resource type");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       UnregistersDescriptorFreeMaterialAndInvalidatesHandle) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Material Instance Unregistration Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::MaterialInstanceDescription description{};

    const Renderer::GpuResourceHandle first =
        renderer.registerMaterialInstance(description);

    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(first));

    const Renderer::GpuResourceHandle second =
        renderer.registerMaterialInstance(description);

    EXPECT_EQ(second.registry_id, first.registry_id);
    EXPECT_EQ(second.index, first.index);
    EXPECT_EQ(second.generation, first.generation + 1U);

    ASSERT_DEATH(
        { renderer.unregisterMaterialInstance(first); },
        "requires a registered material instance handle");

    EXPECT_NO_THROW(renderer.unregisterMaterialInstance(second));
    EXPECT_NO_THROW(renderer.renderFrame());
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsInvalidAndForeignMaterialInstanceHandles) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Invalid Material Instance Removal Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
    };

    ASSERT_DEATH(
        { renderer.unregisterMaterialInstance({}); },
        "requires a registered material instance handle");

    const Renderer::GpuResourceHandle texture =
        renderer.registerTexture(makeTexture(kFirstTexturePixels));

    ASSERT_DEATH(
        { renderer.unregisterMaterialInstance(texture); },
        "requires a registered material instance handle");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       UnregistersTextureBackedMaterialsWithoutRemovingTextures) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    Vulkan::VulkanRenderer renderer{
        "Signum Texture Material Retirement Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        std::nullopt,
        makeTexture(kFirstTexturePixels),
        std::nullopt,
        makeTexturedShaders(),
    };

    const Renderer::GpuResourceHandle texture =
        renderer.registerTexture(makeTexture(kSecondTexturePixels));

    const Renderer::MaterialInstanceDescription description =
        makeTextureMaterialDescription(texture);

    const Renderer::GpuResourceHandle first =
        renderer.registerMaterialInstance(description);

    const Renderer::GpuResourceHandle second =
        renderer.registerMaterialInstance(description);

    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(first));

    const Renderer::GpuResourceHandle third =
        renderer.registerMaterialInstance(description);

    EXPECT_NE(first, third);
    EXPECT_EQ(first.registry_id, third.registry_id);
    EXPECT_EQ(first.index, third.index);
    EXPECT_EQ(first.generation + 1U, third.generation);

    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(second));
    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(third));

    constexpr std::size_t kFrameCount{4U};

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        EXPECT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       UnregistersUniformAndStorageBackedMaterialInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    constexpr std::array types{
        Shader::ShaderResourceType::UniformBuffer,
        Shader::ShaderResourceType::StorageBuffer,
    };

    for (const Shader::ShaderResourceType type : types) {
        Renderer::MaterialBufferBindingInput initial_binding =
            makeInitialBufferBinding();

        initial_binding.type = type;

        const std::array initial_bindings{initial_binding};

        const char *vertex_shader =
            type == Shader::ShaderResourceType::UniformBuffer
                ? "material_update.vert.spv"
                : "material_storage.vert.spv";

        Vulkan::VulkanRenderer renderer{
            "Signum Buffer Material Retirement Test",
            m_Window.value(),
            Renderer::PresentationPreference::VSync,
            false,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            makeShaders(vertex_shader),
            initial_bindings,
        };

        const Renderer::GpuResourceHandle buffer =
            renderer.registerMaterialBuffer(
                std::as_bytes(std::span{kBufferData}), type);

        const Renderer::MaterialInstanceDescription description =
            makeBufferMaterialDescription(buffer, type);

        const Renderer::GpuResourceHandle material =
            renderer.registerMaterialInstance(description);

        ASSERT_NO_THROW(renderer.unregisterMaterialInstance(material));

        ASSERT_DEATH(
            { renderer.unregisterMaterialInstance(material); },
            "requires a registered material instance handle");

        EXPECT_NO_THROW(renderer.renderFrame());
    }
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsRemovalWhileReferencedByDrawInstance) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const Renderer::InitialMeshData initial_mesh = makeInitialMesh();

    Vulkan::VulkanRenderer renderer{
        "Signum Referenced Material Removal Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::GpuResourceHandle mesh =
        renderer.registerMesh(initial_mesh.mesh);

    const Renderer::GpuResourceHandle material =
        renderer.registerMaterialInstance({});

    Renderer::MeshDrawInstance instance{
        .mesh_handle = mesh,
        .model_matrix = Math::identity(),
        .material_handle = {},
    };

    instance.material_handle = material;

    const std::array instances{instance};

    ASSERT_NO_THROW(renderer.setDrawInstances(instances));

    ASSERT_DEATH(
        { renderer.unregisterMaterialInstance(material); },
        "cannot remove a referenced material instance");

    ASSERT_NO_THROW(renderer.setDrawInstances({}));

    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(material));

    EXPECT_NO_THROW(renderer.renderFrame());
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       AcceptsDefaultAndRegisteredMaterialDrawInstances) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const Renderer::InitialMeshData initial_mesh = makeInitialMesh();

    Vulkan::VulkanRenderer renderer{
        "Signum Draw Material Validation Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::GpuResourceHandle mesh =
        renderer.registerMesh(initial_mesh.mesh);

    const Renderer::GpuResourceHandle material =
        renderer.registerMaterialInstance({});

    const std::array instances{
        makeDrawInstance(mesh),
        makeDrawInstance(mesh, material),
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(instances));

    constexpr std::size_t kFrameCount{3U};

    for (std::size_t frame{}; frame < kFrameCount; ++frame) {
        ASSERT_NO_THROW(renderer.renderFrame());
    }

    ASSERT_DEATH(
        { renderer.unregisterMaterialInstance(material); },
        "cannot remove a referenced material instance");

    ASSERT_NO_THROW(renderer.setDrawInstances({}));
    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(material));
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsInvalidAndForeignDrawMaterialHandles) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const Renderer::InitialMeshData initial_mesh = makeInitialMesh();

    Vulkan::VulkanRenderer renderer{
        "Signum Invalid Draw Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::GpuResourceHandle mesh =
        renderer.registerMesh(initial_mesh.mesh);

    const std::array invalid_instances{
        makeDrawInstance(mesh, Renderer::GpuResourceHandle{}),
    };

    ASSERT_DEATH(
        { renderer.setDrawInstances(invalid_instances); },
        "requires registered material instance handles");

    const Renderer::GpuResourceHandle texture =
        renderer.registerTexture(makeTexture(kFirstTexturePixels));

    const std::array foreign_instances{
        makeDrawInstance(mesh, texture),
    };

    ASSERT_DEATH(
        { renderer.setDrawInstances(foreign_instances); },
        "requires registered material instance handles");
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RejectsStaleDrawMaterialHandlesAfterSlotReuse) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const Renderer::InitialMeshData initial_mesh = makeInitialMesh();

    Vulkan::VulkanRenderer renderer{
        "Signum Stale Draw Material Test",
        m_Window.value(),
        Renderer::PresentationPreference::VSync,
        false,
        initial_mesh,
        std::nullopt,
        std::nullopt,
        makeShaders("triangle.vert.spv"),
    };

    const Renderer::GpuResourceHandle mesh =
        renderer.registerMesh(initial_mesh.mesh);

    const Renderer::GpuResourceHandle old_material =
        renderer.registerMaterialInstance({});

    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(old_material));

    const Renderer::GpuResourceHandle new_material =
        renderer.registerMaterialInstance({});

    EXPECT_EQ(old_material.registry_id, new_material.registry_id);
    EXPECT_EQ(old_material.index, new_material.index);
    EXPECT_EQ(old_material.generation + 1U, new_material.generation);

    const std::array stale_instances{
        makeDrawInstance(mesh, old_material),
    };

    ASSERT_DEATH(
        { renderer.setDrawInstances(stale_instances); },
        "requires registered material instance handles");

    const std::array valid_instances{
        makeDrawInstance(mesh, new_material),
    };

    ASSERT_NO_THROW(renderer.setDrawInstances(valid_instances));

    ASSERT_NO_THROW(renderer.setDrawInstances({}));
    ASSERT_NO_THROW(renderer.unregisterMaterialInstance(new_material));
}

TEST_F(VulkanRendererMaterialInstanceIntegrationTests,
       RendersMultipleMaterialsAndDefaultAcrossFrames) {
    if (!m_Window.has_value()) {
        FAIL() << "Material instance test requires an initialized window";
    }

    const Renderer::InitialMeshData initial_mesh = makeTexturedInitialMesh();

    const auto [validation_output, operation_error] =
        captureMixedMaterialValidationOutput([&] -> void {
            Vulkan::VulkanRenderer renderer{
                "Signum Mixed Material Rendering Test",
                m_Window.value(),
                Renderer::PresentationPreference::VSync,
                true,
                initial_mesh,
                makeTexture(kFirstTexturePixels),
                std::nullopt,
                makeTexturedShaders(),
            };

            const Renderer::GpuResourceHandle mesh =
                renderer.registerMesh(initial_mesh.mesh);

            const Renderer::GpuResourceHandle first_texture =
                renderer.registerTexture(makeTexture(kFirstTexturePixels));

            const Renderer::GpuResourceHandle second_texture =
                renderer.registerTexture(makeTexture(kSecondTexturePixels));

            const Renderer::GpuResourceHandle first_material =
                renderer.registerMaterialInstance(
                    makeTextureMaterialDescription(first_texture));

            const Renderer::GpuResourceHandle second_material =
                renderer.registerMaterialInstance(
                    makeTextureMaterialDescription(second_texture));

            Renderer::MeshDrawInstance first =
                makeDrawInstance(mesh, first_material);

            constexpr float kTranslateX = 0.6F;
            constexpr float kTranslateY = 0.5F;

            first.model_matrix = Math::translate(-kTranslateX, 0.0F, 0.0F);

            Renderer::MeshDrawInstance second =
                makeDrawInstance(mesh, second_material);

            second.model_matrix = Math::translate(kTranslateX, 0.0F, 0.0F);

            Renderer::MeshDrawInstance default_material =
                makeDrawInstance(mesh);

            default_material.model_matrix =
                Math::translate(0.0F, kTranslateY, 0.0F);

            const std::array instances{
                first,
                second,
                default_material,
            };

            renderer.setDrawInstances(instances);

            constexpr std::size_t kFrameCount{4U};

            for (std::size_t frame{}; frame < kFrameCount; ++frame) {
                renderer.renderFrame();
            }

            const std::array reordered_instances{
                second,
                default_material,
                first,
            };

            renderer.setDrawInstances(reordered_instances);

            for (std::size_t frame{}; frame < kFrameCount; ++frame) {
                renderer.renderFrame();
            }

            renderer.setDrawInstances({});

            renderer.unregisterMaterialInstance(first_material);
            renderer.unregisterMaterialInstance(second_material);

            for (std::size_t frame{}; frame < kFrameCount; ++frame) {
                renderer.renderFrame();
            }
        });

    EXPECT_TRUE(operation_error.empty())
        << "Mixed-material rendering failed: " << operation_error;

    EXPECT_FALSE(hasUnexpectedMixedMaterialValidationOutput(validation_output))
        << "Unexpected Vulkan validation output:\n"
        << validation_output;
}
