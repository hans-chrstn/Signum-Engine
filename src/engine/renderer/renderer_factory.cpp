#include "renderer_factory.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/renderer_creation_info.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <memory>
#include <utility>

namespace SNE::Engine::Renderer {
    auto createRenderer(RendererCreationInfo creation_info)
        -> std::unique_ptr<IRenderer> {
        if (creation_info.backend != GraphicsBackend::Vulkan) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "Renderer factory requires a recognized graphics backend");
        }

        return std::make_unique<Vulkan::VulkanRenderer>(
            creation_info.application_name, creation_info.window,
            creation_info.presentation_preference,
            creation_info.development_diagnostics_enabled,
            creation_info.initial_mesh, creation_info.initial_texture,
            creation_info.initial_camera,
            std::move(creation_info.initial_graphics_shaders),
            creation_info.initial_buffer_bindings,
            creation_info.initial_texture_bindings,
            creation_info.require_swapchain_transfer_source,
            creation_info.logger);
    }
} // namespace SNE::Engine::Renderer
